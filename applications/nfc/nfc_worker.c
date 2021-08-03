#include "nfc_worker_i.h"
#include <api-hal.h>
#include "nfc_protocols/emv_decoder.h"
#include "nfc_protocols/mifare_ultralight.h"

#define NFC_WORKER_TAG "nfc worker"

/***************************** NFC Worker API *******************************/

NfcWorker* nfc_worker_alloc() {
    NfcWorker* nfc_worker = furi_alloc(sizeof(NfcWorker));
    // Worker thread attributes
    nfc_worker->thread_attr.name = "nfc_worker";
    nfc_worker->thread_attr.stack_size = 8192;
    nfc_worker->callback = NULL;
    nfc_worker->context = NULL;
    // Initialize rfal
    if(!api_hal_nfc_is_busy()) {
        nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    } else {
        nfc_worker_change_state(nfc_worker, NfcWorkerStateBroken);
    }

    return nfc_worker;
}

void nfc_worker_free(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    free(nfc_worker);
}

NfcWorkerState nfc_worker_get_state(NfcWorker* nfc_worker) {
    return nfc_worker->state;
}

void nfc_worker_start(
    NfcWorker* nfc_worker,
    NfcWorkerState state,
    NfcDeviceData* dev_data,
    NfcWorkerCallback callback,
    void* context) {
    furi_assert(nfc_worker);
    furi_assert(dev_data);
    while(nfc_worker->state != NfcWorkerStateReady) {
        osDelay(10);
    }

    nfc_worker->callback = callback;
    nfc_worker->context = context;
    nfc_worker->dev_data = dev_data;
    nfc_worker_change_state(nfc_worker, state);
    nfc_worker->thread = osThreadNew(nfc_worker_task, nfc_worker, &nfc_worker->thread_attr);
}

void nfc_worker_stop(NfcWorker* nfc_worker) {
    furi_assert(nfc_worker);
    if(nfc_worker->state == NfcWorkerStateBroken || nfc_worker->state == NfcWorkerStateReady) {
        return;
    }

    nfc_worker_change_state(nfc_worker, NfcWorkerStateStop);
}

void nfc_worker_change_state(NfcWorker* nfc_worker, NfcWorkerState state) {
    nfc_worker->state = state;
}

/***************************** NFC Worker Thread *******************************/

void nfc_worker_task(void* context) {
    NfcWorker* nfc_worker = context;

    api_hal_power_insomnia_enter();
    api_hal_nfc_exit_sleep();

    if(nfc_worker->state == NfcWorkerStateDetect) {
        nfc_worker_detect(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulate) {
        nfc_worker_emulate(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMVApp) {
        nfc_worker_read_emv_app(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadEMV) {
        nfc_worker_read_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateEMV) {
        nfc_worker_emulate_emv(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateReadMifareUl) {
        nfc_worker_read_mifare_ul(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateEmulateMifareUl) {
        nfc_worker_emulate_mifare_ul(nfc_worker);
    } else if(nfc_worker->state == NfcWorkerStateField) {
        nfc_worker_field(nfc_worker);
    }
    api_hal_nfc_deactivate();
    nfc_worker_change_state(nfc_worker, NfcWorkerStateReady);
    api_hal_power_insomnia_exit();
    osThreadExit();
}

void nfc_worker_detect(NfcWorker* nfc_worker) {
    rfalNfcDevice* dev_list;
    rfalNfcDevice* dev;
    uint8_t dev_cnt;
    NfcDeviceCommomData* result = &nfc_worker->dev_data->nfc_data;

    while(nfc_worker->state == NfcWorkerStateDetect) {
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, true)) {
            // Process first found device
            dev = &dev_list[0];
            result->uid_len = dev->nfcidLen;
            memcpy(result->uid, dev->nfcid, dev->nfcidLen);
            if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCA) {
                result->device = NfcDeviceNfca;
                result->atqa[0] = dev->dev.nfca.sensRes.anticollisionInfo;
                result->atqa[1] = dev->dev.nfca.sensRes.platformInfo;
                result->sak = dev->dev.nfca.selRes.sak;
                if(mf_ul_check_card_type(
                       dev->dev.nfca.sensRes.anticollisionInfo,
                       dev->dev.nfca.sensRes.platformInfo,
                       dev->dev.nfca.selRes.sak)) {
                    result->protocol = NfcDeviceProtocolMifareUl;
                } else if(dev->rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                    result->protocol = NfcDeviceProtocolEMV;
                } else {
                    result->protocol = NfcDeviceProtocolUnknown;
                }
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCB) {
                result->device = NfcDeviceNfcb;
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCF) {
                result->device = NfcDeviceNfcf;
            } else if(dev->type == RFAL_NFC_LISTEN_TYPE_NFCV) {
                result->device = NfcDeviceNfcv;
            }
            // Notify caller and exit
            if(nfc_worker->callback) {
                nfc_worker->callback(nfc_worker->context);
            }
            break;
        }
        osDelay(100);
    }
}

void nfc_worker_emulate(NfcWorker* nfc_worker) {
    NfcDeviceCommomData* data = &nfc_worker->dev_data->nfc_data;
    while(nfc_worker->state == NfcWorkerStateEmulate) {
        if(api_hal_nfc_listen(data->uid, data->uid_len, data->atqa, data->sak, 100)) {
            FURI_LOG_I(NFC_WORKER_TAG, "Reader detected");
        }
        osDelay(10);
    }
}

void nfc_worker_read_emv_app(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    EmvApplication emv_app = {};
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceData* result = nfc_worker->dev_data;

    while(nfc_worker->state == NfcWorkerStateReadEMVApp) {
        memset(&emv_app, 0, sizeof(emv_app));
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
            // Card was found. Check that it supports EMV
            if(dev_list[0].rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
                result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                memcpy(
                    result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);
                result->nfc_data.protocol = NfcDeviceProtocolEMV;

                FURI_LOG_I(NFC_WORKER_TAG, "Send select PPSE command");
                tx_len = emv_prepare_select_ppse(tx_buff);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(NFC_WORKER_TAG, "Error during selection PPSE request: %d", err);
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG, "Select PPSE response received. Start parsing response");
                if(emv_decode_ppse_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Select PPSE responce parced");
                    // Notify caller and exit
                    result->emv_data.aid_len = emv_app.aid_len;
                    memcpy(result->emv_data.aid, emv_app.aid, emv_app.aid_len);
                    if(nfc_worker->callback) {
                        nfc_worker->callback(nfc_worker->context);
                    }
                    break;
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't find pay application");
                    api_hal_nfc_deactivate();
                    continue;
                }
            } else {
                // Can't find EMV card
                FURI_LOG_W(NFC_WORKER_TAG, "Card doesn't support EMV");
                api_hal_nfc_deactivate();
            }
        } else {
            // Can't find EMV card
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find any cards");
            api_hal_nfc_deactivate();
        }
        osDelay(20);
    }
}

void nfc_worker_read_emv(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    EmvApplication emv_app = {};
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceData* result = nfc_worker->dev_data;

    while(nfc_worker->state == NfcWorkerStateReadEMV) {
        memset(&emv_app, 0, sizeof(emv_app));
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 1000, false)) {
            // Card was found. Check that it supports EMV
            if(dev_list[0].rfInterface == RFAL_NFC_INTERFACE_ISODEP) {
                result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
                result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                memcpy(
                    result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);
                result->nfc_data.protocol = NfcDeviceProtocolEMV;

                FURI_LOG_I(NFC_WORKER_TAG, "Send select PPSE command");
                tx_len = emv_prepare_select_ppse(tx_buff);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(NFC_WORKER_TAG, "Error during selection PPSE request: %d", err);
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG, "Select PPSE response received. Start parsing response");
                if(emv_decode_ppse_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Select PPSE responce parced");
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't find pay application");
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(NFC_WORKER_TAG, "Starting application ...");
                tx_len = emv_prepare_select_app(tx_buff, &emv_app);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(
                        NFC_WORKER_TAG, "Error during application selection request: %d", err);
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(
                    NFC_WORKER_TAG,
                    "Select application response received. Start parsing response");
                if(emv_decode_select_app_response(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Card name: %s", emv_app.name);
                    memcpy(result->emv_data.name, emv_app.name, sizeof(emv_app.name));
                } else {
                    FURI_LOG_E(NFC_WORKER_TAG, "Can't read card name");
                    api_hal_nfc_deactivate();
                    continue;
                }
                FURI_LOG_I(NFC_WORKER_TAG, "Starting Get Processing Options command ...");
                tx_len = emv_prepare_get_proc_opt(tx_buff, &emv_app);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err != ERR_NONE) {
                    FURI_LOG_E(
                        NFC_WORKER_TAG, "Error during Get Processing Options command: %d", err);
                    api_hal_nfc_deactivate();
                    continue;
                }
                if(emv_decode_get_proc_opt(rx_buff, *rx_len, &emv_app)) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Card number parsed");
                    memcpy(
                        result->emv_data.number, emv_app.card_number, sizeof(emv_app.card_number));
                    // Notify caller and exit
                    if(nfc_worker->callback) {
                        nfc_worker->callback(nfc_worker->context);
                    }
                    break;
                } else {
                    // Mastercard doesn't give PAN / card number as GPO response
                    // Iterate over all files found in application
                    bool pan_found = false;
                    for(uint8_t i = 0; (i < emv_app.afl.size) && !pan_found; i += 4) {
                        uint8_t sfi = emv_app.afl.data[i] >> 3;
                        uint8_t record_start = emv_app.afl.data[i + 1];
                        uint8_t record_end = emv_app.afl.data[i + 2];

                        // Iterate over all records in file
                        for(uint8_t record = record_start; record <= record_end; ++record) {
                            tx_len = emv_prepare_read_sfi_record(tx_buff, sfi, record);
                            err = api_hal_nfc_data_exchange(
                                tx_buff, tx_len, &rx_buff, &rx_len, false);
                            if(err != ERR_NONE) {
                                FURI_LOG_E(
                                    NFC_WORKER_TAG,
                                    "Error reading application sfi %d, record %d",
                                    sfi,
                                    record);
                            }
                            if(emv_decode_read_sfi_record(rx_buff, *rx_len, &emv_app)) {
                                pan_found = true;
                                break;
                            }
                        }
                    }
                    if(pan_found) {
                        FURI_LOG_I(NFC_WORKER_TAG, "Card PAN found");
                        memcpy(
                            result->emv_data.number,
                            emv_app.card_number,
                            sizeof(emv_app.card_number));
                        // Notify caller and exit
                        if(nfc_worker->callback) {
                            nfc_worker->callback(nfc_worker->context);
                        }
                        break;
                    } else {
                        FURI_LOG_E(NFC_WORKER_TAG, "Can't read card number");
                    }
                    api_hal_nfc_deactivate();
                }
            } else {
                // Can't find EMV card
                FURI_LOG_W(NFC_WORKER_TAG, "Card doesn't support EMV");
                api_hal_nfc_deactivate();
            }
        } else {
            // Can't find EMV card
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find any cards");
            api_hal_nfc_deactivate();
        }
        osDelay(20);
    }
}

void nfc_worker_emulate_emv(NfcWorker* nfc_worker) {
    ReturnCode err;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceCommomData params = {
        .uid = {0xCF, 0x72, 0xd4, 0x40},
        .uid_len = 4,
        .atqa = {0x00, 0x04},
        .sak = 0x20,
        .device = NfcDeviceNfca,
        .protocol = NfcDeviceProtocolEMV,
    };

    while(nfc_worker->state == NfcWorkerStateEmulateEMV) {
        if(api_hal_nfc_listen(params.uid, params.uid_len, params.atqa, params.sak, 300)) {
            FURI_LOG_I(NFC_WORKER_TAG, "POS terminal detected");
            // Read data from POS terminal
            err = api_hal_nfc_data_exchange(NULL, 0, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received Select PPSE");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 1st data exchange: select PPSE");
                api_hal_nfc_deactivate();
                continue;
            }
            FURI_LOG_I(NFC_WORKER_TAG, "Transive SELECT PPSE ANS");
            tx_len = emv_select_ppse_ans(tx_buff);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received Select APP");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 2nd data exchange: select APP");
                api_hal_nfc_deactivate();
                continue;
            }

            FURI_LOG_I(NFC_WORKER_TAG, "Transive SELECT APP ANS");
            tx_len = emv_select_app_ans(tx_buff);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received PDOL");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 3rd data exchange: receive PDOL");
                api_hal_nfc_deactivate();
                continue;
            }

            FURI_LOG_I(NFC_WORKER_TAG, "Transive PDOL ANS");
            tx_len = emv_get_proc_opt_ans(tx_buff);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received PDOL");
            }
            api_hal_nfc_deactivate();
        } else {
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find reader");
        }
        osDelay(20);
    }
}

void nfc_worker_read_mifare_ul(NfcWorker* nfc_worker) {
    ReturnCode err;
    rfalNfcDevice* dev_list;
    uint8_t dev_cnt = 0;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    MifareUlDevice mf_ul_read;
    NfcDeviceData* result = nfc_worker->dev_data;

    while(nfc_worker->state == NfcWorkerStateReadMifareUl) {
        api_hal_nfc_deactivate();
        memset(&mf_ul_read, 0, sizeof(mf_ul_read));
        if(api_hal_nfc_detect(&dev_list, &dev_cnt, 300, false)) {
            if(dev_list[0].type == RFAL_NFC_LISTEN_TYPE_NFCA &&
               mf_ul_check_card_type(
                   dev_list[0].dev.nfca.sensRes.anticollisionInfo,
                   dev_list[0].dev.nfca.sensRes.platformInfo,
                   dev_list[0].dev.nfca.selRes.sak)) {
                // Get Mifare Ultralight version
                FURI_LOG_I(NFC_WORKER_TAG, "Found Mifare Ultralight tag. Reading tag version");
                tx_len = mf_ul_prepare_get_version(tx_buff);
                err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
                if(err == ERR_NONE) {
                    mf_ul_parse_get_version_response(rx_buff, &mf_ul_read);
                    FURI_LOG_I(
                        NFC_WORKER_TAG,
                        "Mifare Ultralight Type: %d, Pages: %d",
                        mf_ul_read.type,
                        mf_ul_read.pages_to_read);
                } else if(err == ERR_TIMEOUT) {
                    FURI_LOG_W(
                        NFC_WORKER_TAG,
                        "Card doesn't respond to GET VERSION command. Setting default read parameters");
                    err = ERR_NONE;
                    mf_ul_set_default_version(&mf_ul_read);
                    // Reinit device
                    api_hal_nfc_deactivate();
                    if(!api_hal_nfc_detect(&dev_list, &dev_cnt, 300, false)) {
                        FURI_LOG_E(NFC_WORKER_TAG, "Lost connection. Restarting search");
                        continue;
                    }
                } else {
                    FURI_LOG_E(
                        NFC_WORKER_TAG,
                        "Error getting Mifare Ultralight version. Error code: %d",
                        err);
                    continue;
                }

                if(mf_ul_read.support_fast_read) {
                    FURI_LOG_I(NFC_WORKER_TAG, "Reading pages ...");
                    tx_len = mf_ul_prepare_fast_read(tx_buff, 0x00, mf_ul_read.pages_to_read - 1);
                    if(api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                        FURI_LOG_E(NFC_WORKER_TAG, "Failed reading pages");
                        continue;
                    } else {
                        mf_ul_parse_fast_read_response(
                            rx_buff, 0x00, mf_ul_read.pages_to_read - 1, &mf_ul_read);
                    }

                    FURI_LOG_I(NFC_WORKER_TAG, "Reading signature ...");
                    tx_len = mf_ul_prepare_read_signature(tx_buff);
                    if(api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                        FURI_LOG_W(NFC_WORKER_TAG, "Failed reading signature");
                        memset(mf_ul_read.data.signature, 0, sizeof(mf_ul_read.data.signature));
                    } else {
                        mf_ul_parse_read_signature_response(rx_buff, &mf_ul_read);
                    }

                    FURI_LOG_I(NFC_WORKER_TAG, "Reading 3 counters ...");
                    for(uint8_t i = 0; i < 3; i++) {
                        tx_len = mf_ul_prepare_read_cnt(tx_buff, i);
                        if(api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                            FURI_LOG_W(NFC_WORKER_TAG, "Failed reading Counter %d", i);
                            mf_ul_read.data.counter[i] = 0;
                        } else {
                            mf_ul_parse_read_cnt_response(rx_buff, i, &mf_ul_read);
                        }
                    }

                    FURI_LOG_I(NFC_WORKER_TAG, "Checking tearing flags ...");
                    for(uint8_t i = 0; i < 3; i++) {
                        tx_len = mf_ul_prepare_check_tearing(tx_buff, i);
                        if(api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                            FURI_LOG_E(NFC_WORKER_TAG, "Error checking tearing flag %d", i);
                            mf_ul_read.data.tearing[i] = MF_UL_TEARING_FLAG_DEFAULT;
                        } else {
                            mf_ul_parse_check_tearing_response(rx_buff, i, &mf_ul_read);
                        }
                    }
                } else {
                    // READ card with READ command (4 pages at a time)
                    for(uint8_t page = 0; page < mf_ul_read.pages_to_read; page += 4) {
                        FURI_LOG_I(NFC_WORKER_TAG, "Reading pages %d - %d ...", page, page + 3);
                        tx_len = mf_ul_prepare_read(tx_buff, page);
                        if(api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false)) {
                            FURI_LOG_E(
                                NFC_WORKER_TAG, "Read pages %d - %d failed", page, page + 3);
                            continue;
                        } else {
                            mf_ul_parse_read_response(rx_buff, page, &mf_ul_read);
                        }
                    }
                }

                // Fill result data
                result->nfc_data.uid_len = dev_list[0].dev.nfca.nfcId1Len;
                result->nfc_data.atqa[0] = dev_list[0].dev.nfca.sensRes.anticollisionInfo;
                result->nfc_data.atqa[1] = dev_list[0].dev.nfca.sensRes.platformInfo;
                result->nfc_data.sak = dev_list[0].dev.nfca.selRes.sak;
                result->nfc_data.protocol = NfcDeviceProtocolMifareUl;
                memcpy(
                    result->nfc_data.uid, dev_list[0].dev.nfca.nfcId1, result->nfc_data.uid_len);
                result->mf_ul_data = mf_ul_read.data;

                // Notify caller and exit
                if(nfc_worker->callback) {
                    nfc_worker->callback(nfc_worker->context);
                }
                break;
            } else {
                FURI_LOG_W(NFC_WORKER_TAG, "Tag does not support Mifare Ultralight");
            }
        } else {
            FURI_LOG_W(NFC_WORKER_TAG, "Can't find any tags");
        }
        osDelay(100);
    }
}

void nfc_worker_emulate_mifare_ul(NfcWorker* nfc_worker) {
    ReturnCode err;
    uint8_t tx_buff[255] = {};
    uint16_t tx_len = 0;
    uint8_t* rx_buff;
    uint16_t* rx_len;
    NfcDeviceData* data = nfc_worker->dev_data;

    while(nfc_worker->state == NfcWorkerStateEmulateMifareUl) {
        if(api_hal_nfc_listen(
               data->nfc_data.uid,
               data->nfc_data.uid_len,
               data->nfc_data.atqa,
               data->nfc_data.sak,
               1000)) {
            FURI_LOG_I(NFC_WORKER_TAG, "Hello my dudes");
            // Prepare version answer
            tx_len = sizeof(data->mf_ul_data.version);
            memcpy(tx_buff, &data->mf_ul_data.version, tx_len);
            err = api_hal_nfc_data_exchange(tx_buff, tx_len, &rx_buff, &rx_len, false);
            if(err == ERR_NONE) {
                FURI_LOG_I(NFC_WORKER_TAG, "Received 1st message:");
                for(uint16_t i = 0; i < *rx_len; i++) {
                    printf("%02X ", rx_buff[i]);
                }
                printf("\r\n");
            } else {
                FURI_LOG_E(NFC_WORKER_TAG, "Error in 1st data exchange: select PPSE");
                api_hal_nfc_deactivate();
                continue;
            }
        }
        FURI_LOG_W(NFC_WORKER_TAG, "Hello my dudes");
        osDelay(10);
    }
}

void nfc_worker_field(NfcWorker* nfc_worker) {
    api_hal_nfc_field_on();
    while(nfc_worker->state == NfcWorkerStateField) {
        osDelay(50);
    }
    api_hal_nfc_field_off();
}
