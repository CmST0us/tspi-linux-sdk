CURRENT_TIME=`date +%m%d%H%M%S`
CURL_CMD="/data/duer/curl"
SERVER_ADDR="http://10.201.42.53:8234/corelog"

CUID=`vendor_storage -r VENDOR_SN_ID`
VERSION=`head -1 /etc/X180_version.ini`
FILE_PREFIX="${CUID}_${VERSION}_${CURRENT_TIME}_"

upload_file() {
    if [[ ! -f $1 ]]; then
        exit 1
    fi

    ORG_FILENAME=`echo $1 | cut -f5 -d"/"`
    cp $1 /tmp/${FILE_PREFIX}${ORG_FILENAME}

    export LD_LIBRARY_PATH="/data/duer/lib32"
    CMD="${CURL_CMD} -X POST ${SERVER_ADDR} -F file=@/tmp/${FILE_PREFIX}${ORG_FILENAME}"
    `${CMD}`
}

upload_file /data/duer/dcssdk.log
upload_file /data/duer/duer_link.log
upload_file /data/duer/speechsdk.log
