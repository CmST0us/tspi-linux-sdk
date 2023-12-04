// Copyright 2019 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EASYMEDIA_MPP_ENCODER_H
#define EASYMEDIA_MPP_ENCODER_H

#include "encoder.h"
#include "mpp_inc.h"
#include "mpp_rc_api.h"
#include "rk_venc_cfg.h"

namespace easymedia
{

    // A encoder which call the mpp interface directly.
    // Mpp is always video process module.
    class MPPEncoder : public VideoEncoder
    {
      public:
        MPPEncoder();
        virtual ~MPPEncoder();

        virtual bool Init() override;

        // sync encode the raw input buffer to output buffer
        virtual int Process(const std::shared_ptr<MediaBuffer>& input, std::shared_ptr<MediaBuffer>& output,
                            std::shared_ptr<MediaBuffer> extra_output) override;

        virtual int SendInput(const std::shared_ptr<MediaBuffer>&) override;
        virtual std::shared_ptr<MediaBuffer> FetchOutput() override;

        // Encoder statistics enable switch. After enabling, calculate the
        // actual bps and actual fps every theoretical fps period.
        // Input parameters:
        //   value = true, enable encoder statistics.
        //   value = false, disable encoding statistics.
        void set_statistics_switch(bool value);
        int get_statistics_bps();
        int get_statistics_fps();

#ifdef MPP_SUPPORT_HW_OSD
        int OsdPaletteSet(uint32_t* ptl_data);
        int OsdRegionSet(OsdRegionData* region_data);
        int OsdRegionGet(OsdRegionData* region_data);
#endif

        // Set sei info by userdata.
        int SetUserData(const char* data, uint16_t len);
        void ClearUserData();
        void RestartUserData();
        void EnableUserDataAllFrame(bool value);

        // for updating roi regions config.
        int RoiUpdateRegions(EncROIRegion* regions, int region_cnt);

        virtual void QueryChange(uint32_t change, void* value, int32_t size);

      protected:
        MppCodingType coding_type;
        uint32_t output_mb_flags;
        std::string rc_api_brief_name;
        // call before Init()
        void SetMppCodeingType(MppCodingType type);
        virtual bool CheckConfigChange(std::pair<uint32_t, std::shared_ptr<ParameterBuffer>>)
        {
            return true;
        }
        // Control before encoding.
        int EncodeControl(int cmd, void* param);

        virtual int PrepareMppFrame(const std::shared_ptr<MediaBuffer>& input, std::shared_ptr<MediaBuffer>& mdinfo,
                                    MppFrame& frame);
        virtual int PrepareMppPacket(std::shared_ptr<MediaBuffer>& output, MppPacket& packet);
        virtual int PrepareMppExtraBuffer(std::shared_ptr<MediaBuffer> extra_output, MppBuffer& buffer);
        int Process(MppFrame frame, MppPacket& packet, MppBuffer& mv_buf);

      private:
        std::shared_ptr<MPPContext> mpp_ctx;

        // Statistics switch
        bool encoder_sta_en;
        // Statistics variable
        uint32_t encoded_bps;
        float encoded_fps;

        size_t stream_size_1s;
        size_t frame_cnt_1s;
        int64_t last_ts;
        int64_t cur_ts;

#ifdef MPP_SUPPORT_HW_OSD
        MppEncOSDData osd_data;
#endif // MPP_SUPPORT_HW_OSD

        // for roi regions config.
        MppEncROICfg roi_cfg;

#define MPP_ENCODER_USERDATA_MAX_SIZE 1024
        char userdata[MPP_ENCODER_USERDATA_MAX_SIZE];
        uint16_t userdata_len;
        uint16_t userdata_frame_id;
        uint8_t userdata_all_frame_en;
        MppEncUserData mpp_ud;

        friend class MPPMJPEGConfig;
        friend class MPPCommonConfig;
    };

} // namespace easymedia

#endif // EASYMEDIA_MPP_ENCODER_H
