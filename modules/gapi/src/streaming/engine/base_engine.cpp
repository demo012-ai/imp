// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.
//
// Copyright (C) 2021 Intel Corporation

#include <opencv2/gapi/streaming/onevpl_data_provider_interface.hpp>
#include "streaming/engine/base_engine.hpp"
#include "streaming/vpl/vpl_accel_policy.hpp"
#include "logger.hpp"

namespace cv {
namespace gapi {
namespace wip {

VPLProcessingEngine::VPLProcessingEngine(std::unique_ptr<VPLAccelerationPolicy>&& accel) :
    acceleration_policy(std::move(accel)) {
}

VPLProcessingEngine::~VPLProcessingEngine() {
    GAPI_LOG_INFO(nullptr, "destroyed");
}

VPLProcessingEngine::ExecutionStatus VPLProcessingEngine::process(mfxSession session) {
    auto sess_it = sessions.find(session);
    if (sess_it == sessions.end()) {

        // TODO remember the last session status
        return ExecutionStatus::Processed;
    }

    session_ptr processing_session = sess_it->second;
    ExecutionData& exec_data = execution_table[session];

    GAPI_LOG_DEBUG(nullptr, "[" << session <<"] start op id: " << exec_data.op_id);
    ExecutionStatus status = execute_op(pipeline.at(exec_data.op_id), *processing_session);
    size_t old_op_id = exec_data.op_id++;
    if (exec_data.op_id == pipeline.size())
    {
        exec_data.op_id = 0;
    }
    GAPI_LOG_DEBUG(nullptr, "[" << session <<"] finish op id: " << old_op_id <<
                                    ", " << processing_session->error_code_to_str() <<
                                    ", " << VPLProcessingEngine::status_to_string(status) <<
                                    ", next op id: " << exec_data.op_id);

    if (status == ExecutionStatus::Failed) {

        GAPI_LOG_WARNING(nullptr, "Operation for session: " << session <<
                                  ", " << VPLProcessingEngine::status_to_string(status) <<
                                  " - remove it");
        sessions.erase(sess_it);
        execution_table.erase(session);
    }

    if(status == ExecutionStatus::Processed) {
        sessions.erase(sess_it);
        execution_table.erase(session);
    }

    return status;
}

const char * VPLProcessingEngine::status_to_string(ExecutionStatus status)
{
    switch(status) {
        case ExecutionStatus::Continue: return "CONTINUE";
        case ExecutionStatus::Processed: return "PROCESSED";
        case ExecutionStatus::Failed: return "FAILED";
        default:
            return "UNKNOWN";
    }
}

VPLProcessingEngine::ExecutionStatus VPLProcessingEngine::execute_op(operation_t& op, EngineSession& sess)
{
     return op(sess);
}

size_t VPLProcessingEngine::get_ready_frames_count() const
{
    return ready_frames.size();
}

void VPLProcessingEngine::get_frame(Data &data)
{
    data = ready_frames.front();
    ready_frames.pop();
}

const VPLAccelerationPolicy* VPLProcessingEngine::get_accel() const {
    return acceleration_policy.get();
}

VPLAccelerationPolicy* VPLProcessingEngine::get_accel() {
    return const_cast<VPLAccelerationPolicy*>(static_cast<const VPLProcessingEngine*>(this)->get_accel());
}


// Read encoded stream from file
mfxStatus ReadEncodedStream(mfxBitstream &bs, std::shared_ptr<IDataProvider>& data_provider) {

    if (!data_provider) {
        return MFX_ERR_MORE_DATA;
    }

    mfxU8 *p0 = bs.Data;
    mfxU8 *p1 = bs.Data + bs.DataOffset;
    if (bs.DataOffset > bs.MaxLength - 1) {
        return MFX_ERR_NOT_ENOUGH_BUFFER;
    }
    if (bs.DataLength + bs.DataOffset > bs.MaxLength) {
        return MFX_ERR_NOT_ENOUGH_BUFFER;
    }
    for (mfxU32 i = 0; i < bs.DataLength; i++) {
        *(p0++) = *(p1++);
    }
    bs.DataOffset = 0;
    bs.DataLength += (mfxU32)data_provider->provide_data(bs.MaxLength - bs.DataLength, bs.Data + bs.DataLength);
    if (bs.DataLength == 0)
        return MFX_ERR_MORE_DATA;

    return MFX_ERR_NONE;
}
} // namespace wip
} // namespace gapi
} // namespace cv
