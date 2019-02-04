#include "services.h"
#include "sdk_common.h"
#include "ble_srv_common.h"
#include "boards.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "aes.h"


static uint32_t
_bleAddService (ble_service_t * p_service, service_uuid_t service_t)
{
  uint32_t err_code;
  ble_uuid_t ble_uuid;

  ble_uuid128_t base_uuid = { SERVICE_UUID_BASE };
  err_code = sd_ble_uuid_vs_add (&base_uuid, &p_service->uuid_type);
  VERIFY_SUCCESS (err_code);

  ble_uuid.type = p_service->uuid_type;
  ble_uuid.uuid = service_t.uuid_handle.uuid;
  // Add the custom service to the system
  err_code =
    sd_ble_gatts_service_add (BLE_GATTS_SRVC_TYPE_PRIMARY, &ble_uuid,
			      &p_service->service_handle);
  VERIFY_SUCCESS (err_code);


  return NRF_SUCCESS;
}


static uint32_t
_bleAddCharacteristic (ble_service_t * p_service, service_char_t char_t,
		       prop_attr_t prop, attr_perm_t attr_perm,
		       attr_char_t attr_char)
{

  ble_gatts_char_md_t char_md;
  ble_gatts_attr_t attr_char_value;
  ble_uuid_t ble_uuid;
  ble_gatts_attr_md_t attr_md;

  // characteristic metadata
  memset (&char_md, 0, sizeof (char_md));

  char_md.char_props.read = prop.read;
  char_md.char_props.write = prop.write;
  char_md.char_props.notify = prop.notify;
  char_md.p_char_user_desc = NULL;
  char_md.p_char_pf = NULL;
  char_md.p_user_desc_md = NULL;
  char_md.p_cccd_md = NULL;
  char_md.p_sccd_md = NULL;

  // characteristic uuid
  ble_uuid.type = p_service->uuid_type;
  ble_uuid.uuid = char_t.uuid16;

  // attribute metadata
  memset (&attr_md, 0, sizeof (attr_md));

  switch (attr_perm.perm_read)
    {
    case 0x01:
      BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS (&attr_md.read_perm);
      break;

    case 0x02:
      BLE_GAP_CONN_SEC_MODE_SET_OPEN (&attr_md.read_perm);
      break;

    case 0x03:
      BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM (&attr_md.read_perm);
      break;

    case 0x04:
      BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM (&attr_md.read_perm);
      break;

    case 0x05:
      BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM (&attr_md.read_perm);
      break;

    case 0x06:
      BLE_GAP_CONN_SEC_MODE_SET_SIGNED_NO_MITM (&attr_md.read_perm);
      break;

    case 0x07:
      BLE_GAP_CONN_SEC_MODE_SET_SIGNED_WITH_MITM (&attr_md.read_perm);
      break;
    }

  switch (attr_perm.perm_write)
    {
    case 0x01:
      BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS (&attr_md.write_perm);
      break;

    case 0x02:
      BLE_GAP_CONN_SEC_MODE_SET_OPEN (&attr_md.write_perm);
      break;

    case 0x03:
      BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM (&attr_md.write_perm);
      break;

    case 0x04:
      BLE_GAP_CONN_SEC_MODE_SET_ENC_WITH_MITM (&attr_md.write_perm);
      break;

    case 0x05:
      BLE_GAP_CONN_SEC_MODE_SET_LESC_ENC_WITH_MITM (&attr_md.write_perm);
      break;

    case 0x06:
      BLE_GAP_CONN_SEC_MODE_SET_SIGNED_NO_MITM (&attr_md.write_perm);
      break;

    case 0x07:
      BLE_GAP_CONN_SEC_MODE_SET_SIGNED_WITH_MITM (&attr_md.write_perm);
      break;
    }

  attr_md.vloc = BLE_GATTS_VLOC_STACK;
  attr_md.rd_auth = 0;
  attr_md.wr_auth = 0;
  attr_md.vlen = 0;

  // attribute data
  memset (&attr_char_value, 0, sizeof (attr_char_value));

  attr_char_value.p_uuid = &ble_uuid;
  attr_char_value.p_attr_md = &attr_md;
  attr_char_value.init_offs = 0;

  if (prop.read == 1)
    {
      attr_char_value.init_len = sizeof (attr_char.attr_char_a);
      attr_char_value.max_len = sizeof (attr_char.attr_char_a);
      attr_char_value.p_value = attr_char.attr_char_a;
    }
  else if (prop.write == 1)
    {
      attr_char_value.init_len = 0;
      attr_char_value.max_len = 4;
      attr_char_value.p_value = NULL;
    }
  else
    {
      attr_char_value.init_len = 0;
      attr_char_value.max_len = 0;
      attr_char_value.p_value = NULL;
    }


  if (prop.write == 1)
    {
      switch (char_t.char_id)
	{
	case SERVICE_X_CHAR_B_ID:
	  return sd_ble_gatts_characteristic_add (p_service->service_handle,
						  &char_md,
						  &attr_char_value,
						  &p_service->
						  char_xa_handles);
	  break;
	case SERVICE_Y_CHAR_B_ID:
	  return sd_ble_gatts_characteristic_add (p_service->service_handle,
						  &char_md,
						  &attr_char_value,
						  &p_service->
						  char_ya_handles);
	  break;
	default:
	  return sd_ble_gatts_characteristic_add (p_service->service_handle,
						  &char_md,
						  &attr_char_value,
						  &p_service->
						  char_default_handles);
	}
    }
  else
    {
      return sd_ble_gatts_characteristic_add (p_service->service_handle,
					      &char_md,
					      &attr_char_value,
					      &p_service->
					      char_default_handles);
    }
}



uint8_t d1[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void
_writeHandler_X (uint16_t conn_handle, ble_service_t * p_service,
		 uint8_t data_rec, uint8_t offset)
{

  uint8_t d2[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  const uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
    0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
  };

  uint8_t output[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  };

  d1[offset] = data_rec;

  NRF_LOG_INFO ("Value %x %x %x %x", d1[0], d1[1], d1[2], d1[3]);

  if (offset == 3)
    {

      AES128_ECB_encrypt (d1, key, output);
      NRF_LOG_INFO ("Value of ciphertext  %x %x %x %x", output[0], output[1],
		    output[2], output[3]);

      AES128_ECB_decrypt (output, key, d2);
      NRF_LOG_INFO ("Value of plaintext %x %x %x %x", d2[0], d2[1], d2[2],
		    d2[3]);
    }
}

static void
_writeHandler_Y (uint16_t conn_handle, ble_service_t * p_service,
		 uint8_t data_rec, uint8_t offset)
{

  NRF_LOG_INFO ("data received from service Y %x ", data_rec);

}

static void
on_write (ble_service_t * p_service, ble_evt_t const *p_ble_evt)
{
  ble_gatts_evt_write_t const *p_evt_write =
    &p_ble_evt->evt.gatts_evt.params.write;

  if ((p_evt_write->len == 4)
      && (p_evt_write->handle == p_service->char_xa_handles.value_handle))
    {
      _writeHandler_X (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[0], 0);
      _writeHandler_X (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[1], 1);
      _writeHandler_X (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[2], 2);
      _writeHandler_X (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[3], 3);
    }

  if ((p_evt_write->len == 4)
      && (p_evt_write->handle == p_service->char_ya_handles.value_handle))
    {
      _writeHandler_Y (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[0], 0);
      _writeHandler_Y (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[1], 1);
      _writeHandler_Y (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[2], 2);
      _writeHandler_Y (p_ble_evt->evt.gap_evt.conn_handle, p_service,
		       p_evt_write->data[3], 3);
    }
}


void
bleServiceOnBleEvt (ble_evt_t const *p_ble_evt, void *p_context)
{
  ble_service_t *p_service = (ble_service_t *) p_context;

  switch (p_ble_evt->header.evt_id)
    {
    case BLE_GATTS_EVT_WRITE:
      on_write (p_service, p_ble_evt);
      break;

    default:
      // No implementation needed.
      break;
    }
}


uint32_t
bleServiceInit (ble_service_t * p_service, service_uuid_t service_t,
		service_char_t * char_t, prop_attr_t * attr_prop,
		attr_perm_t * attr_perm, attr_char_t * attr_char)
{
  uint32_t err_code;


  // Add service.
  err_code = _bleAddService (p_service, service_t);
  VERIFY_SUCCESS (err_code);


  if (service_t.service_id == 0x0001)
    {
      // Add characteristics.
      for (int i = 0; i < MAX_CHARACTERISTIC_X; i++)
	{
	  err_code =
	    _bleAddCharacteristic (p_service, char_t[i], attr_prop[i],
				   attr_perm[i], attr_char[i]);
	  VERIFY_SUCCESS (err_code);
	}
    }

  if (service_t.service_id == 0x0002)
    {
      // Add characteristics.
      for (int i = 0; i < MAX_CHARACTERISTIC_Y; i++)
	{
	  err_code =
	    _bleAddCharacteristic (p_service, char_t[i], attr_prop[i],
				   attr_perm[i], attr_char[i]);
	  VERIFY_SUCCESS (err_code);
	}
    }

  return NRF_SUCCESS;

}