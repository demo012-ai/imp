// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#include "../precomp.hpp"
#include "layers_common.hpp"

namespace cv { namespace dnn {

class LayerNormLayerImpl CV_FINAL : public LayerNormLayer
{
public:
    LayerNormLayerImpl(const LayerParams& params)
    {
        setParamsFrom(params);

        // standard attr
        axis = params.get<int>("axis", 0);
        epsilon = params.get<float>("epsilon", 1e-5);

        // opencv attr
        hasBias = params.get<bool>("hasBias", false);
    }

    virtual bool supportBackend(int backendId) CV_OVERRIDE
    {
        return backendId == DNN_BACKEND_OPENCV;
    }

    virtual bool getMemoryShapes(const std::vector<MatShape> &inputs,
                                 const int requiredOutputs,
                                 std::vector<MatShape> &outputs,
                                 std::vector<MatShape> &internals) const CV_OVERRIDE
    {
        // check shapes of weight and bias if existed
        // inputs >= 2 (X and Weight are requested, bias is optional)
        CV_CheckGE(inputs.size(), 2ull, "LayerNorm: require at least two inputs (x, weight)");
        CV_CheckLE(inputs.size(), 3ull, "LayerNorm: have at most three inputs (x, weight, bias)");

        auto x_shape = inputs[0];
        int x_ndims = static_cast<int>(x_shape.size());

        auto w_shape = inputs[1];
        // if axis == last_dim, scale and b are both 1d tensor (represented as 2d mat nx1)
        int w_ndims = static_cast<int>(w_shape.size());
        w_ndims = (axis == x_ndims - 1 && w_ndims == 2) ? w_ndims - 1 : w_ndims;
        CV_CheckEQ(x_ndims - axis, w_ndims, "LayerNorm: shape of weight does not match with given axis and shape of input");
        for (int i = 0; i < w_ndims; ++i)
            CV_CheckEQ(x_shape[axis+i], w_shape[i], "LayerNorm: weight dimensions does not match with input dimensions");
        if (hasBias)
        {
            auto b_shape = inputs[2];
            CV_CheckEQ(w_shape.size(), b_shape.size(), "LayerNorm: shape of weight does not match with shape of bias");
            for (size_t i = 0; i < w_shape.size(); ++i)
                CV_CheckEQ(w_shape[i], b_shape[i], "LayerNorm: bias dimensions does not match with weight dimensions");
        }

        // only one output is needed; Mean & InvStdDev are not needed
        // in inference and should beomitted in onnx importer
        outputs.assign(1, inputs[0]);
        return false;
    }

    class LayerNormInvoker : public ParallelLoopBody
    {
    public:
        const Mat* src, *scale, *bias;
        Mat *dst;

        float epsilon;
        int nstripes;

        int total;
        int stripeSize;
        int normSize;

        LayerNormInvoker() : src(0), scale(0), bias(0), dst(0), epsilon(0.f), nstripes(0), total(0), stripeSize(0), normSize(0) { }

        static void run(const Mat* src, const Mat* scale, const Mat* b, Mat* dst, int axis, float epsilon, int nstripes)
        {
            CV_Assert_N( src->isContinuous(), dst->isContinuous(), src->type() == CV_32F, src->type() == dst->type());

            LayerNormInvoker p;

            p.src = src;
            p.scale = scale;
            p.bias = b;
            p.dst = dst;

            p.epsilon = epsilon;
            p.nstripes = nstripes;

            auto dstShape = shape(*dst);
            p.total = std::accumulate(dstShape.begin(), dstShape.begin() + axis, 1, std::multiplies<int>());
            p.stripeSize = (p.total + nstripes - 1) / nstripes;
            p.normSize = std::accumulate(dstShape.begin() + axis, dstShape.end(), 1, std::multiplies<int>());

            parallel_for_(Range(0, nstripes), p, nstripes);
        }

        void operator()(const Range& r) const CV_OVERRIDE
        {
            int stripeStart = r.start * stripeSize;
            int stripeEnd = std::min(r.end * stripeSize, total);

            float *dstData = (float *)dst->data;
            const float *srcData = (const float *)src->data;
            const float *scaleData = (const float *)scale->data;
            const float *biasData = nullptr;
            if (bias != nullptr)
                biasData = (const float *)bias->data;

            for (int ofs = stripeStart; ofs < stripeEnd; ++ofs)
            {
                const float* first = srcData + ofs * normSize;
                float* dst_first = dstData + ofs * normSize;

                float mean = 0;
                float mean_square = 0;
                for (int h = 0; h < normSize; ++h) {
                    mean += first[h];
                    mean_square += first[h] * first[h];
                }
                mean /= normSize;
                mean_square = std::sqrt(mean_square / normSize - mean * mean + epsilon);
                for (int h = 0; h < normSize; ++h) {
                    if (biasData == nullptr) {
                        dst_first[h] = (first[h] - mean) / mean_square * scaleData[h];
                    } else {
                        dst_first[h] = (first[h] - mean) / mean_square * scaleData[h] + biasData[h];
                    }
                }
            }
        }
    };

    void forward(InputArrayOfArrays inputs_arr, OutputArrayOfArrays outputs_arr, OutputArrayOfArrays internals_arr) CV_OVERRIDE
    {
        std::vector<Mat> inputs, outputs;
        inputs_arr.getMatVector(inputs);
        outputs_arr.getMatVector(outputs);
        const int nstripes = getNumThreads();

        if (hasBias)
        {
            LayerNormInvoker::run(&inputs[0], &inputs[1], &inputs[2], &outputs[0], axis, epsilon, nstripes);
        }
        else
        {
            LayerNormInvoker::run(&inputs[0], &inputs[1], nullptr, &outputs[0], axis, epsilon, nstripes);
        }
    }
};

Ptr<LayerNormLayer> LayerNormLayer::create(const LayerParams& params)
{
    return makePtr<LayerNormLayerImpl>(params);
}

}} // cv::dnn
