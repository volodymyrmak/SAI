#include "sai.h"
#include "stub_sai.h"
#include "assert.h"

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    static int32_t next_lag_id = 1;
    sai_status_t status;
    status = stub_create_object(SAI_OBJECT_TYPE_LAG, next_lag_id++, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Cannot create a LAG OID!\n");
        return status;
    }

    printf("Create LAG: 0x%lX\n", *lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    printf("Remove LAG: 0x%lX\n", lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    printf("Set LAG_ATTR: 0x%lX\n", lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("Get LAG_ATTR: 0x%lX\n", lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    printf("Create LAG_MEMBER: 0x%lX\n", *lag_member_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    printf("Remove LAG_MEMBER: 0x%lX\n", lag_member_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    printf("Set LAG_MEMBER_ATTR: 0x%lX\n", lag_member_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    printf("Get LAG_MEMBER_ATTR: 0x%lX\n", lag_member_id);
    return SAI_STATUS_SUCCESS;
}

const sai_lag_api_t lag_api = {
    stub_create_lag,
    stub_remove_lag,
    stub_set_lag_attribute,
    stub_get_lag_attribute,
    stub_create_lag_member,
    stub_remove_lag_member,
    stub_set_lag_member_attribute,
    stub_get_lag_member_attribute
};
