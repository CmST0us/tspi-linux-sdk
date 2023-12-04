/*
 *	Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 *	Author: chad.ma <chad.ma@rock-chips.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "update_recv.h"

DWORD m_fwOffset;
FILE* pImgFile;
long long  m_fileSize = 0;
bool bCheck = false;
char mnt_point[256];

char* try_mount_point[10] = {
	"/udisk/",
	"/mnt/udisk/",
	"/mnt/usb_storage/",
	"/mnt/sdcard/",
	"/mnt/external_sd/",

	NULL,
};

long long GetFwSize(char* fwFilePath)
{
	struct stat statBuf;
	char szName[256];
	memset(szName, 0, sizeof(szName));
	strcpy(szName,fwFilePath);

	memset(mnt_point, 0, sizeof(mnt_point));

	if (access(szName, F_OK) == 0) {
		if (stat(szName, &statBuf) < 0){
			printf("%s : stat fail, try \n", szName);
			return -1;
		}

		strcpy(mnt_point, szName);
		m_fileSize = statBuf.st_size;
		return m_fileSize;
	} else {
		//try
		int i = 0;
		while(try_mount_point[i] != NULL) {
			memset(szName, 0, sizeof(szName));
			strcpy(szName, try_mount_point[i]);
			strcat(szName, UPDATE_IMG);
			printf("===>>>>We will try: %s \n", szName);

			if (access(szName, F_OK) == 0) {
				//find valid mount point.
				if (stat(szName, &statBuf) < 0){
					printf("%s : stat fail, try  again\n", szName);
					return -1;
				}

				strcpy(mnt_point, szName);
				m_fileSize = statBuf.st_size;
				printf("Get Fw total Size = %lld Bytes\n", m_fileSize);
				return m_fileSize;
			}

			i++;
		}

		printf("*** No find valid mount point for 'update.img' ***\n");
	}

	return m_fileSize;
}

bool GetData(long long dwOffset,DWORD dwSize,PBYTE lpBuffer)
{
	if ( dwOffset<0 || dwSize==0 )
		return false;

	if ( dwOffset + dwSize > m_fileSize)
		return false;

	//lseek64(pImgFile,dwOffset,SEEK_SET);
	fseek(pImgFile, dwOffset, SEEK_SET);
	UINT uiActualRead;
	uiActualRead = fread(lpBuffer, 1, dwSize, pImgFile);
	//uiActualRead = read(pImgFile, lpBuffer, dwSize);
	if (dwSize != uiActualRead)
		return false;

	return true;
}

DWORD GetFwOffset(FILE* pImageFile)
{
	int ret;
	long long ulFwSize;
	STRUCT_RKIMAGE_HEAD imageHead;
	fseeko(pImageFile, 0, SEEK_SET);
	ret = fread((PBYTE)(&imageHead),1,sizeof(STRUCT_RKIMAGE_HEAD),pImageFile);
	//ret = read(pImageFile, (PBYTE)(&imageHead), sizeof(STRUCT_RKIMAGE_HEAD));
	if (ret != sizeof(STRUCT_RKIMAGE_HEAD)){
		printf("%s<%d> Read update.img failed!\n", __func__, __LINE__);
		fclose(pImageFile);
		return -1;
	}

	if ( imageHead.uiTag!=0x57464B52 ) {
		bCheck = false;
		return -1;
	}

	return imageHead.dwFWOffset;
}

static void ShowLog(char* fwImg, bool isCheck)
{
	printf("===========================\n");
	if (!isCheck)
		printf("  update %s start\n", fwImg);
	else
		printf("  Check %s start\n", fwImg);
}

int WriteFwData(char* imgPath, char* fwName)
{
	bool bRet;
	long long fwSize = 0;
	long long dwFwOffset;
	STRUCT_RKIMAGE_HDR rkImageHead;
	int idx,iHeadSize;
	FILE* pRecvNode = NULL;
	long long fileBufferSize;
	long long EntryStartOffset;
	UINT uiWriteByte = 0;
	long long uiEntryOffset = 0;
	PBYTE pBuffer = NULL;
	UINT uiBufferSize = LBA_TRANSFER_SIZE;
	printf("### %s() Enter \n", __func__);

	ShowLog(fwName, false);

	fwSize = GetFwSize(imgPath);
	if (fwSize < 0) {
		printf("GetFwSize %s Error\n", imgPath);
		return -2;
	}

	if (mnt_point[0] == 0) {
		printf("### Error : Not find update.img ### \n");
		return -2;
	}

	pImgFile = fopen(mnt_point, "rb");
	if (pImgFile == NULL)
	{
		printf("%s<%d> Open %s failed! Error:%s\n", __func__, __LINE__,
			   mnt_point, strerror(errno));
		return -2;
	}

	m_fwOffset = GetFwOffset(pImgFile);
	if (bCheck == false && m_fwOffset < 0) {
		printf("GetFwOffset %s Error\n", imgPath);
		return -2;
	}
	printf("m_fwOffset = 0x%08x \n", m_fwOffset);

	dwFwOffset = m_fwOffset;
	iHeadSize = sizeof(STRUCT_RKIMAGE_HDR);
	bRet = GetData(dwFwOffset, iHeadSize, (PBYTE)&rkImageHead);
	if ( !bRet )
	{
		printf("### GetData error ###\n");
		return -2;
	}

	if (rkImageHead.item_count <= 0)
	{
		printf("### ERROR:DownloadImage-->No Found item ###\n");
		return -2;
	}

	for (idx = 0; idx < rkImageHead.item_count; idx++) {
		if (strcmp(rkImageHead.item[idx].name, fwName) != 0)
			continue;
		else
			break;
	}

	if (idx == rkImageHead.item_count) {
		printf("## Not found %s in update.img ##\n", fwName);
		goto ERR;
	}

	pRecvNode = fopen(DEV_RECOVERY_NODE, "wb");
	if (pRecvNode  == NULL)
	{
		printf("%s<%d> Open %s failed! Error:%s\n", __func__, __LINE__,
			   DEV_RECOVERY_NODE, strerror(errno));
		return -1;
	}

	//lseek(pRecvNode, 0, SEEK_SET);
	fseek(pRecvNode, 0, SEEK_SET);

	for (idx = 0; idx < rkImageHead.item_count; idx++ )
	{
		if (strcmp(rkImageHead.item[idx].name, fwName) != 0)
			continue;

		if (rkImageHead.item[idx].file[55]=='H') {
			fileBufferSize = *((DWORD *)(&rkImageHead.item[idx].file[56]));
			fileBufferSize <<= 32;
			fileBufferSize += rkImageHead.item[idx].size;
		}
		else
			fileBufferSize = rkImageHead.item[idx].size;

		printf("fileBufferSize = 0x%08x \n", fileBufferSize);

		if (fileBufferSize > 0) {
			DWORD dwFWOffset;
			dwFWOffset = m_fwOffset;

			if (rkImageHead.item[idx].file[50]=='H') {
				EntryStartOffset = *((DWORD *)(rkImageHead.item[idx].file[51]));
				EntryStartOffset <<= 32;
				EntryStartOffset += rkImageHead.item[idx].offset;
				EntryStartOffset += dwFWOffset;
			} else {
				EntryStartOffset = dwFWOffset;
				EntryStartOffset += rkImageHead.item[idx].offset;
			}

			pBuffer = (PBYTE)malloc(uiBufferSize * sizeof(BYTE));
			if (pBuffer == NULL) {
				printf("Error, No enough memory!!!\n");
				return -1;
			}

			while ( fileBufferSize > 0 ) {
				memset(pBuffer,0,uiBufferSize);

				if ( fileBufferSize < uiBufferSize ) {
					uiWriteByte = fileBufferSize;
				} else {
					uiWriteByte = uiBufferSize;
				}

				bRet = GetData(dwFWOffset + rkImageHead.item[idx].offset + uiEntryOffset,
								uiWriteByte,pBuffer);
				if ( !bRet ) {
					printf("ERROR:RKA_File_Download-->GetFileData failed\n");
					goto ERR;
				}

				size_t sizeWr = fwrite(pBuffer, 1, uiWriteByte, pRecvNode);
				//size_t sizeWr = write(recvNode_fd, pBuffer, uiWriteByte);
				if (sizeWr != uiWriteByte) {
					printf("### Write Error !!!\n");
					goto ERR;
				}

				printf("=");
				fileBufferSize -= uiWriteByte;
				uiEntryOffset += uiWriteByte;
			}
		}

		printf("\n\n");
		printf("================== Update %s Success ==============\n", fwName);

	}

	if (pRecvNode != NULL) {
		fclose(pRecvNode);
		pRecvNode = NULL;
	}
	if (pImgFile != NULL) {
		fclose(pImgFile);
		pImgFile = NULL;
	}

	return 0;

ERR:
	if (pBuffer) {
		free(pBuffer);
		pBuffer = NULL;
	}

	if (pRecvNode != NULL) {
		fclose(pRecvNode);
		pRecvNode = NULL;
	}
	if (pImgFile != NULL) {
		fclose(pImgFile);
		pImgFile = NULL;
	}
	printf("\n\n");
	printf("================== Update %s Fail ==============\n", fwName);
	return -1;
}

bool CheckFwData(char* imgPath, char* fwName)
{
	bool bRet;
	long long dwFwOffset;
	STRUCT_RKIMAGE_HDR rkImageHead;
	int idx,iHeadSize;
	FILE* pRecvNode = NULL;
	long long fileBufferSize;
	long long EntryStartOffset;
	UINT uiReadByte = 0;
	long long uiEntryOffset = 0;
	PBYTE pBufferFromImg = NULL;
	PBYTE pBufferFromFlash = NULL;
	UINT uiBufferSize = LBA_TRANSFER_SIZE;

	printf("### %s() Enter \n", __func__);

	ShowLog(fwName, true);

	if (m_fileSize < 0) {
		printf("get %s file size Error\n", imgPath);
		return false;
	}

	if (mnt_point[0] == 0) {
		printf("### Error : Not find update.img ### \n");
		return false;
	}

	pImgFile = fopen(mnt_point, "rb");
	if (pImgFile == NULL)
	{
		printf("%s<%d> Open %s failed! Error:%s\n", __func__, __LINE__,
			   mnt_point, strerror(errno));
		return false;
	}

	if (bCheck == false && m_fwOffset < 0) {
		printf("GetFwOffset %s Error\n", imgPath);
		return false;
	}
	printf("m_fwOffset = 0x%08x \n", m_fwOffset);

	dwFwOffset = m_fwOffset;
	iHeadSize = sizeof(STRUCT_RKIMAGE_HDR);
	bRet = GetData(dwFwOffset, iHeadSize, (PBYTE)&rkImageHead);
	if ( !bRet )
	{
		printf("### GetData error ###\n");
		return false;
	}

	pRecvNode = fopen(DEV_RECOVERY_NODE, "rb");
	if (pRecvNode  == NULL)
	{
		printf("%s<%d> Open %s failed! Error:%s\n", __func__, __LINE__,
			   DEV_RECOVERY_NODE, strerror(errno));
		return false;
	}

	//lseek(pRecvNode, 0, SEEK_SET);
	fseek(pRecvNode, 0, SEEK_SET);

	for (idx = 0; idx < rkImageHead.item_count; idx++ )
	{
		if (strcmp(rkImageHead.item[idx].name, fwName) != 0)
			continue;

		if (rkImageHead.item[idx].file[55]=='H') {
			fileBufferSize = *((DWORD *)(&rkImageHead.item[idx].file[56]));
			fileBufferSize <<= 32;
			fileBufferSize += rkImageHead.item[idx].size;
		}
		else
			fileBufferSize = rkImageHead.item[idx].size;

		printf("fileBufferSize = 0x%08x \n", fileBufferSize);

		if (fileBufferSize > 0) {
			DWORD dwFWOffset;
			dwFWOffset = m_fwOffset;

			if (rkImageHead.item[idx].file[50]=='H') {
				EntryStartOffset = *((DWORD *)(rkImageHead.item[idx].file[51]));
				EntryStartOffset <<= 32;
				EntryStartOffset += rkImageHead.item[idx].offset;
				EntryStartOffset += dwFWOffset;
			} else {
				EntryStartOffset = dwFWOffset;
				EntryStartOffset += rkImageHead.item[idx].offset;
			}

			pBufferFromImg = (PBYTE)malloc(uiBufferSize * sizeof(BYTE));
			if (pBufferFromImg == NULL) {
				printf("Error, No enough memory!!!\n");
				return false;
			}

			pBufferFromFlash = (PBYTE)malloc(uiBufferSize * sizeof(BYTE));
			if (pBufferFromFlash == NULL) {
				printf("Error, No enough memory!!!\n");
				return false;
			}

			while ( fileBufferSize > 0 ) {
				memset(pBufferFromImg, 0, uiBufferSize);
				memset(pBufferFromFlash, 0, uiBufferSize);

				if ( fileBufferSize < uiBufferSize ) {
					uiReadByte = fileBufferSize;
				} else {
					uiReadByte = uiBufferSize;
				}

				bRet = GetData(dwFWOffset + rkImageHead.item[idx].offset + uiEntryOffset,
								uiReadByte,pBufferFromImg);
				if ( !bRet ) {
					printf("ERROR:RKA_File_Download-->GetFileData failed\n");
					goto ERR;
				}

				size_t sizeRd = fread(pBufferFromFlash, 1, uiReadByte, pRecvNode);
				//size_t sizeRd = read(recvNode_fd, pBuffer, uiWriteByte);
				if (sizeRd != uiReadByte) {
					printf("### Read from flash Error !!!\n");
					goto ERR;
				}

				if (memcmp(pBufferFromImg, pBufferFromFlash, uiReadByte) != 0) {
					goto ERR;
				}

				printf("=");
				fileBufferSize -= uiReadByte;
				uiEntryOffset += uiReadByte;
			}
		}

		printf("\n\n");
		printf("================== Check %s Success ==============\n", fwName);
	}

	if (pRecvNode != NULL) {
		fclose(pRecvNode);
		pRecvNode = NULL;
	}
	if (pImgFile != NULL) {
		fclose(pImgFile);
		pImgFile = NULL;
	}

	return true;

ERR:
	if (pBufferFromImg) {
		free(pBufferFromImg);
		pBufferFromImg = NULL;
	}
	if (pBufferFromFlash) {
		free(pBufferFromFlash);
		pBufferFromFlash = NULL;
	}

	if (pRecvNode != NULL) {
		fclose(pRecvNode);
		pRecvNode = NULL;
	}
	if (pImgFile != NULL) {
		fclose(pImgFile);
		pImgFile = NULL;
	}
	printf("\n\n");
	printf("================== Check %s Fail ==================\n", fwName);
	return false;
}


