/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_tdls_main.c
 *
 * TDLS core function definitions
 */

#include "wlan_tdls_main.h"
#include "wlan_tdls_cmds_process.h"
#include "wlan_tdls_peer.h"

QDF_STATUS tdls_psoc_obj_create_notification(struct wlan_objmgr_psoc *psoc,
					     void *arg_list)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *tdls_soc_obj;

	tdls_soc_obj = qdf_mem_malloc(sizeof(*tdls_soc_obj));
	if (!tdls_soc_obj) {
		tdls_err("Failed to allocate memory for tdls object");
		return QDF_STATUS_E_NOMEM;
	}

	tdls_soc_obj->soc = psoc;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_TDLS,
						       (void *)tdls_soc_obj,
						       QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to attach psoc tdls component");
		qdf_mem_free(tdls_soc_obj);
	}

	tdls_notice("TDLS obj attach to psoc successfully");

	return status;
}

QDF_STATUS tdls_psoc_obj_destroy_notification(struct wlan_objmgr_psoc *psoc,
					      void *arg_list)
{
	QDF_STATUS status;
	struct tdls_soc_priv_obj *tdls_soc_obj;

	tdls_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						WLAN_UMAC_COMP_TDLS);
	if (!tdls_soc_obj) {
		tdls_err("Failed to get tdls obj in psoc");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_TDLS,
						       tdls_soc_obj);

	if (QDF_IS_STATUS_ERROR(status))
		tdls_err("Failed to detach psoc tdls component");

	qdf_mem_free(tdls_soc_obj);

	return status;
}

static QDF_STATUS tdls_vdev_init(struct tdls_vdev_priv_obj *vdev_obj)
{
	uint8_t i;
	struct tdls_config_params *config;
	struct tdls_user_config *user_config;
	struct tdls_soc_priv_obj *soc_obj;

	soc_obj = wlan_vdev_get_tdls_soc_obj(vdev_obj->vdev);
	if (!soc_obj) {
		tdls_err("tdls soc obj NULL");
		return QDF_STATUS_E_FAILURE;
	}

	config = &vdev_obj->threshold_config;
	user_config = &soc_obj->tdls_configs;
	config->tx_period_t = user_config->tdls_tx_states_period;
	config->tx_packet_n = user_config->tdls_tx_pkt_threshold;
	config->discovery_tries_n = user_config->tdls_max_discovery_attempt;
	config->idle_timeout_t = user_config->tdls_idle_timeout;
	config->idle_packet_n = user_config->tdls_idle_pkt_threshold;
	config->rssi_trigger_threshold =
		user_config->tdls_rssi_trigger_threshold;
	config->rssi_teardown_threshold =
		user_config->tdls_rssi_teardown_threshold;
	config->rssi_delta = user_config->tdls_rssi_delta;

	for (i = 0; i < WLAN_TDLS_PEER_LIST_SIZE; i++) {
		qdf_list_create(&vdev_obj->peer_list[i],
				WLAN_TDLS_PEER_SUB_LIST_SIZE);
	}
	qdf_mc_timer_init(&vdev_obj->peer_update_timer, QDF_TIMER_TYPE_SW,
			  NULL, vdev_obj);
	qdf_mc_timer_init(&vdev_obj->peer_discovery_timer, QDF_TIMER_TYPE_SW,
			  NULL, vdev_obj);
	return QDF_STATUS_SUCCESS;
}

static void tdls_vdev_deinit(struct tdls_vdev_priv_obj *vdev_obj)
{
	qdf_mc_timer_stop(&vdev_obj->peer_update_timer);
	qdf_mc_timer_stop(&vdev_obj->peer_discovery_timer);

	qdf_mc_timer_destroy(&vdev_obj->peer_update_timer);
	qdf_mc_timer_destroy(&vdev_obj->peer_discovery_timer);

	tdls_peer_idle_timers_destroy(vdev_obj);
	tdls_free_peer_list(vdev_obj);
}

QDF_STATUS tdls_vdev_obj_create_notification(struct wlan_objmgr_vdev *vdev,
					     void *arg)
{
	QDF_STATUS status;
	struct tdls_vdev_priv_obj *tdls_vdev_obj;

	tdls_notice("tdls vdev mode %d", wlan_vdev_mlme_get_opmode(vdev));
	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE &&
	    wlan_vdev_mlme_get_opmode(vdev) != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	/* TODO: Add concurrency check */

	tdls_vdev_obj = qdf_mem_malloc(sizeof(*tdls_vdev_obj));
	if (!tdls_vdev_obj) {
		tdls_err("Failed to allocate memory for tdls vdev object");
		return QDF_STATUS_E_NOMEM;
	}

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
						       WLAN_UMAC_COMP_TDLS,
						       (void *)tdls_vdev_obj,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		tdls_err("Failed to attach vdev tdls component");
		qdf_mem_free(tdls_vdev_obj);
		goto out;
	}
	tdls_vdev_obj->vdev = vdev;
	status = tdls_vdev_init(tdls_vdev_obj);
	if (QDF_IS_STATUS_ERROR(status))
		goto out;

	tdls_notice("tdls object attach to vdev successfully");
out:
	return status;
}

QDF_STATUS tdls_vdev_obj_destroy_notification(struct wlan_objmgr_vdev *vdev,
					      void *arg)
{
	QDF_STATUS status;
	void *tdls_vdev_obj;

	tdls_notice("tdls vdev mode %d", wlan_vdev_mlme_get_opmode(vdev));
	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE &&
	    wlan_vdev_mlme_get_opmode(vdev) != QDF_P2P_CLIENT_MODE)
		return QDF_STATUS_SUCCESS;

	tdls_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							WLAN_UMAC_COMP_TDLS);
	if (!tdls_vdev_obj) {
		tdls_err("Failed to get tdls vdev object");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_vdev_component_obj_detach(vdev,
						       WLAN_UMAC_COMP_TDLS,
						       tdls_vdev_obj);
	if (QDF_IS_STATUS_ERROR(status))
		tdls_err("Failed to detach vdev tdls component");

	tdls_vdev_deinit(tdls_vdev_obj);
	qdf_mem_free(tdls_vdev_obj);

	return status;
}

QDF_STATUS tdls_process_cmd(struct scheduler_msg *msg)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!msg || !msg->bodyptr) {
		tdls_err("msg: 0x%p, bodyptr: 0x%p", msg, msg->bodyptr);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (msg->type) {
	case TDLS_CMD_TX_ACTION:
		break;
	case TDLS_CMD_ADD_STA:
		tdls_process_add_peer(msg->bodyptr);
		break;
	case TDLS_CMD_CHANGE_STA:
		tdls_process_update_peer(msg->bodyptr);
		break;
	case TDLS_CMD_ENABLE_LINK:
		break;
	case TDLS_CMD_DISABLE_LINK:
		tdls_process_del_peer(msg->bodyptr);
		break;
	case TDLS_CMD_CONFIG_FORCE_PEER:
		break;
	case TDLS_CMD_REMOVE_FORCE_PEER:
		break;
	case TDLS_CMD_STATS_UPDATE:
		break;
	case TDLS_CMD_CONFIG_UPDATE:
		break;
	default:
		break;
	}

	return status;
}
