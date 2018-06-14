/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Define VDEV MLME init/deinit APIs
 */

#ifndef _WLAN_VDEV_MLME_MAIN_H_
#define _WLAN_VDEV_MLME_MAIN_H_

#ifdef CMN_VDEV_MLME_SM_ENABLE
/**
 * wlan_vdev_mlme_init - Initializes MLME component
 *
 * Registers callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_vdev_mlme_init(void);

/**
 * wlan_vdev_mlme_deinit - Uninitializes MLME component
 *
 * Unregisters callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
QDF_STATUS wlan_vdev_mlme_deinit(void);

#else

/**
 * wlan_vdev_mlme_init - Initializes MLME component
 *
 * Registers callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
static inline QDF_STATUS wlan_vdev_mlme_init(void)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_vdev_mlme_deinit - Uninitializes MLME component
 *
 * Unregisters callbacks with object manager for create/destroy
 *
 * Return: SUCCESS on successful registration
 *         FAILURE, if registration fails
 */
static inline QDF_STATUS wlan_vdev_mlme_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif

