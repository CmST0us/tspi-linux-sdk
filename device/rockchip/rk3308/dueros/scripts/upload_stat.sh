CURRENT_TIME=`date +%H%M`
UPLOAD_TIME="1000"
CURL_CMD="/data/duer/curl"
SERVER_ADDR="http://10.201.42.53:8234/testdata"

if [[ "${CURRENT_TIME}" != "${UPLOAD_TIME}" ]]; then
    exit 0
fi

CUID=`vendor_storage -r VENDOR_SN_ID | cut -b15-18`
VERSION=`head -1 /etc/X180_version.ini`
CURRENT_DATE=`date +%m%d`
FILE_PREFIX="${CUID}_${VERSION}_${CURRENT_DATE}_"
UPTIME=`uptime`

upload_file() {
    if [[ ! -f $1 ]]; then
        exit 1
    fi

    ORG_FILENAME=`echo $1 | cut -f5 -d"/"`
    COPY_FILENAME="/tmp/${FILE_PREFIX}${ORG_FILENAME}"
    if [[ -f ${COPY_FILENAME} ]] ; then
        exit 0
    fi

    cp $1 ${COPY_FILENAME}

    export LD_LIBRARY_PATH="/data/duer/lib32"
    CMD="${CURL_CMD} -X POST ${SERVER_ADDR} -F file=@${COPY_FILENAME}"
    `${CMD}`

    #rm ${COPY_FILENAME}
}

upload_file /data/duer/test/counts_alsa_audio_main_service.txt
upload_file /data/duer/test/counts_duer_linux.txt
upload_file /data/duer/test/top_alsa_audio_main_service.txt
upload_file /data/duer/test/top_duer_linux.txt

echo ${UPTIME} > /data/duer/test/uptime.txt
upload_file /data/duer/test/uptime.txt
