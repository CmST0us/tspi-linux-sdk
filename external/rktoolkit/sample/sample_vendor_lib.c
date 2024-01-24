 /*
  * Copyright 2020 Fuzhou Rockchip Electronics Co., Ltd. All rights reserved.
  * Use of this source code is governed by a BSD-style license that can be
  * found in the LICENSE file.
  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vendor_storage.h"

int main(int argc, char *argv[])
{
	char data_0[128] = {0};
	char data_1[128] = {0};

	rkvendor_read(VENDOR_SN_ID, data_0, sizeof(data_0)/sizeof(data_0[0]));
	fprintf(stderr, "DEMO-%d: read vendor sn: %s\n", __LINE__, data_0);

	rkvendor_write(VENDOR_SN_ID, "DEMO-Serial No: 1234567890", strlen("DEMO-Serial No: 1234567890"));
	rkvendor_read(VENDOR_SN_ID, data_1, sizeof(data_1)/sizeof(data_1[0]));
	fprintf(stderr, "DEMO-%d: read vendor sn: %s\n", __LINE__, data_1);

	rkvendor_write(VENDOR_SN_ID, data_0, sizeof(data_0)/sizeof(data_0[0]));

	return EXIT_SUCCESS;
}
