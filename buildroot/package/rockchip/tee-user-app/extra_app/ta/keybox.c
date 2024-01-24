// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2021 Rockchip Electronics Co. Ltd.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include <tee_api_defines.h>
#include "ta_keybox.h"
#include <tee_api.h>
#include "../rk_public_api/rk_trng_api.h"

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
				    TEE_Param  params[4], void **sess_ctx)
{
	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;
	(void)&param_types;

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
}

static TEE_Result TA_TouchtHandle(uint32_t type, uint8_t *id, int len,
			   uint32_t flags, TEE_ObjectHandle *handle)
{
	TEE_Result res = TEE_SUCCESS;

	res = TEE_OpenPersistentObject(type, id, len, flags, handle);
	if (res == TEE_SUCCESS)
		return res;

	res = TEE_CreatePersistentObject(type, id, len, flags,
					 TEE_HANDLE_NULL, NULL, 0,
					 handle);
	return res;
}

static uint32_t js_hash(uint32_t hash, uint8_t *buf, int len)
{
	int i;

	for (i = 0; i < len; i++)
		hash ^= ((hash << 5) + buf[i] + (hash >> 2));

	return hash;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the parameters
 * comes from normal world.
 */
static uint32_t g_hash = 0;
static uint32_t g_ver = 0;
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
				      uint32_t param_types, TEE_Param params[4])
{
	/* Unused parameters */
	(void)&sess_ctx;
	uint32_t exp_param_types;
	TEE_Result res = TEE_SUCCESS;
	uint32_t flags = TEE_DATA_FLAG_ACCESS_READ |
			 TEE_DATA_FLAG_ACCESS_WRITE |
			 TEE_DATA_FLAG_ACCESS_WRITE_META;
	TEE_ObjectHandle ob_handle;
	uint8_t id[] = "enc_key";
	uint32_t count = 0;
	void *buf = NULL;

	buf = TEE_Malloc(64, 0);
	if (!buf)
		return TEE_ERROR_OUT_OF_MEMORY;

	switch (cmd_id) {
	case TA_KEY_RNG:
		exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
						  TEE_PARAM_TYPE_NONE,
						  TEE_PARAM_TYPE_NONE,
						  TEE_PARAM_TYPE_NONE);
		if (param_types != exp_param_types)
			return TEE_ERROR_BAD_PARAMETERS;

		res = rk_get_trng(buf, params[0].memref.size);
		if (res != TEE_SUCCESS) {
			EMSG("rk_get_trng failed with code 0x%x", res);
			goto out;
		}

		g_hash = js_hash(0x47c6a7e6, buf, params[0].memref.size);
		TEE_MemMove(params[0].memref.buffer, buf, params[0].memref.size);
		break;
	case TA_KEY_VER:
		exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						  TEE_PARAM_TYPE_NONE,
						  TEE_PARAM_TYPE_NONE,
						  TEE_PARAM_TYPE_NONE);
		if (param_types != exp_param_types)
			return TEE_ERROR_BAD_PARAMETERS;

		if (g_hash && (g_hash == params[0].value.a)) {
			//IMSG("******** PASS");
			g_ver = 1;
		} else {
			//IMSG("******** failed (g_hash = %x)", g_hash);
			g_ver = 0;
		}

		break;
	case TA_KEY_READ:
		exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
						  TEE_PARAM_TYPE_VALUE_INPUT,
						  TEE_PARAM_TYPE_NONE,
						  TEE_PARAM_TYPE_NONE);
		if (param_types != exp_param_types)
			return TEE_ERROR_BAD_PARAMETERS;

		if (params[0].memref.size > 64)
			return TEE_ERROR_BAD_PARAMETERS;

		if (!g_ver) {
			res = rk_get_trng(buf, params[0].memref.size);
			if (res != TEE_SUCCESS)
				EMSG("rk_get_trng failed with code 0x%x", res);
		} else {
			res = TEE_OpenPersistentObject(params[1].value.a,
						       id, sizeof(id), flags,
						       &ob_handle);

			if (res != TEE_SUCCESS) {
				EMSG("OpenPersistentObject ERR: 0x%x.", res);
				break;
			}

			res = TEE_ReadObjectData(ob_handle, buf,
						 params[0].memref.size, &count);
			if (res != TEE_SUCCESS) {
				EMSG("ReadObjectData ERR: 0x%x.", res);
				goto out_r;
			}

			if (count != params[0].memref.size)
				res = TEE_ERROR_OUT_OF_MEMORY;
out_r:
			TEE_CloseObject(ob_handle);
		}
		TEE_MemMove(params[0].memref.buffer, buf, params[0].memref.size);

		break;
	case TA_KEY_WRITE:
		exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						  TEE_PARAM_TYPE_VALUE_INPUT,
						  TEE_PARAM_TYPE_NONE,
						  TEE_PARAM_TYPE_NONE);
		if (param_types != exp_param_types)
			return TEE_ERROR_BAD_PARAMETERS;

		if (params[0].memref.size > 64)
			return TEE_ERROR_BAD_PARAMETERS;

		TEE_MemMove(buf, params[0].memref.buffer, params[0].memref.size);

		/* Try to get handle if node exist */
		res = TA_TouchtHandle(params[1].value.a, id, sizeof(id),
				      flags, &ob_handle);

		if (res != TEE_SUCCESS)
			break;

		res = TEE_SeekObjectData(ob_handle, 0, TEE_DATA_SEEK_SET);
		if (res != TEE_SUCCESS) {
			EMSG("SeekObjectData ERR: 0x%x.", res);
			goto out_w;
		}

		res = TEE_WriteObjectData(ob_handle, buf, params[0].memref.size);
		if (res != TEE_SUCCESS) {
			EMSG("WriteObjectData ERR: 0x%x.", res);
			goto out_w;
		}
out_w:
		TEE_CloseObject(ob_handle);
		break;
	default:
		break;
	}

out:
	if (buf)
		TEE_Free(buf);

	return res;
}
