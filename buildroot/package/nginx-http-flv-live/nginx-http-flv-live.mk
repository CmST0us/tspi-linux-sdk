################################################################################
#
# nginx-http-flv-live
#
################################################################################

NGINX_HTTP_FLV_LIVE_VERSION = v1.2.8
NGINX_HTTP_FLV_LIVE_SITE = $(call github,winshining,nginx-http-flv-module,$(NGINX_HTTP_FLV_LIVE_VERSION))

$(eval $(generic-package))
