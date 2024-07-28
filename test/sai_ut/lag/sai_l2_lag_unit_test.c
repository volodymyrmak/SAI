#include <stdio.h>
#include <assert.h>
#include "sai.h"


#define MAX_LAG_PORT_COUNT 32
#define LAG_ATTR_COUNT 1
#define LAG_MEMBER_ATTR_COUNT 2

const char* test_profile_get_value(
    _In_ sai_switch_profile_id_t profile_id,
    _In_ const char* variable)
{
    return 0;
}

int test_profile_get_next_value(
    _In_ sai_switch_profile_id_t profile_id,
    _Out_ const char** variable,
    _Out_ const char** value)
{
    return -1;
}

const service_method_table_t test_services = {
    test_profile_get_value,
    test_profile_get_next_value
};

void test_create_and_remove_simple_lag(sai_lag_api_t *lag_api)
{
    sai_object_id_t lag_oid;
    // Create LAG
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag(&lag_oid, 0, NULL));
    // Remove LAG
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag(lag_oid));
}

void test_create_lag_members_and_link_to_lag(sai_lag_api_t *lag_api, sai_object_id_t *port_list)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_member_id1;
    sai_object_id_t lag_member_id2;
    sai_attribute_t attrs[LAG_MEMBER_ATTR_COUNT];

    // Create LAG
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag(&lag_id, 0, NULL));

    // Create LAG_MEMBER1
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs[1].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.oid = port_list[0];
    attrs[1].value.oid = lag_id;
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag_member(&lag_member_id1, LAG_MEMBER_ATTR_COUNT, attrs));

    // Create LAG_MEMBER2
    attrs[0].value.oid = port_list[1];
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag_member(&lag_member_id2, LAG_MEMBER_ATTR_COUNT, attrs));

    // Remove LAG_MEMBERs
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag_member(lag_member_id1));
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag_member(lag_member_id2));

    // Remove LAG
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag(lag_id));
}

void test_try_remove_lag_with_active_lag_members(sai_lag_api_t *lag_api, sai_object_id_t *port_list)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_member_id;
    sai_attribute_t attrs[LAG_MEMBER_ATTR_COUNT];

    // Create LAG
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag(&lag_id, 0, NULL));

    // Create LAG_MEMBER1
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs[1].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.oid = port_list[0];
    attrs[1].value.oid = lag_id;
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag_member(&lag_member_id, LAG_MEMBER_ATTR_COUNT, attrs));

    // Try to remove the LAG with active members and expect failure
    assert(SAI_STATUS_FAILURE == lag_api->remove_lag(lag_id));

    // Remove LAG_MEMBER
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag_member(lag_member_id));

    // Remove LAG
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag(lag_id));
}

void test_create_lag_and_get_expected_attr_values(sai_lag_api_t *lag_api, sai_object_id_t *port_list)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_member_id1;
    sai_object_id_t lag_member_id2;
    sai_attribute_t attrs[LAG_MEMBER_ATTR_COUNT];

    // Create LAG
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag(&lag_id, 0, NULL));

    // Create LAG_MEMBER1
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs[1].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    attrs[0].value.oid = port_list[0];
    attrs[1].value.oid = lag_id;
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag_member(&lag_member_id1, LAG_MEMBER_ATTR_COUNT, attrs));

    // Create LAG_MEMBER2
    attrs[0].value.oid = port_list[1];
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag_member(&lag_member_id2, LAG_MEMBER_ATTR_COUNT, attrs));

    // Get PORT_LIST and compare with expected
    sai_attribute_t lag_attr[LAG_ATTR_COUNT];
    lag_attr[0].id = SAI_LAG_ATTR_PORT_LIST;
    assert(SAI_STATUS_SUCCESS == lag_api->get_lag_attribute(lag_id, LAG_ATTR_COUNT, lag_attr));

    for (int i = 0; i < lag_attr[0].value.objlist.count; ++i)
    {
        assert(lag_attr[0].value.objlist.list[i] == port_list[i]);
    }

    // Release an allocated memory
    free(lag_attr[0].value.objlist.list);

    // Remove LAG_MEMBERs
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag_member(lag_member_id1));
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag_member(lag_member_id2));

    // Remove LAG
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag(lag_id));
}

void test_create_lag_member_and_get_expected_attr_values(sai_lag_api_t *lag_api, sai_object_id_t *port_list)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_member_id;
    sai_attribute_t expected_attrs[LAG_MEMBER_ATTR_COUNT];

    // Create LAG
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag(&lag_id, 0, NULL));

    // Create LAG_MEMBER1
    expected_attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    expected_attrs[1].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    expected_attrs[0].value.oid = port_list[0];
    expected_attrs[1].value.oid = lag_id;
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag_member(&lag_member_id, LAG_MEMBER_ATTR_COUNT, expected_attrs));

    // Get attr values
    sai_attribute_t attrs[LAG_MEMBER_ATTR_COUNT];
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs[1].id = SAI_LAG_MEMBER_ATTR_LAG_ID;
    assert(SAI_STATUS_SUCCESS == lag_api->get_lag_member_attribute(lag_member_id, LAG_MEMBER_ATTR_COUNT, attrs));

    // Compare got values with expected ones
    assert(port_list[0] == attrs[0].value.oid);
    assert(lag_id       == attrs[1].value.oid);

    // Remove LAG_MEMBER
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag_member(lag_member_id));

    // Remove LAG
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag(lag_id));
}

void test_try_create_lag_member_without_mandatory_attrs(sai_lag_api_t *lag_api, sai_object_id_t *port_list)
{
    sai_object_id_t lag_id;
    sai_object_id_t lag_member_id;
    sai_attribute_t attrs[LAG_MEMBER_ATTR_COUNT];

    // Create LAG
    assert(SAI_STATUS_SUCCESS == lag_api->create_lag(&lag_id, 0, NULL));

    // Create LAG_MEMBER1 without LAG_ID and expect failure
    attrs[0].id = SAI_LAG_MEMBER_ATTR_PORT_ID;
    attrs[0].value.oid = port_list[0];
    assert(SAI_STATUS_ATTR_NOT_IMPLEMENTED_MAX == lag_api->create_lag_member(&lag_member_id, LAG_MEMBER_ATTR_COUNT, attrs));

    // Remove LAG
    assert(SAI_STATUS_SUCCESS == lag_api->remove_lag(lag_id));
}


int main()
{
    sai_status_t    status;
    sai_lag_api_t   *lag_api;
    
    // Sample PORT_LIST
    sai_object_id_t port_list[MAX_LAG_PORT_COUNT] = {1, 2, 3, 4, 5};

    status = sai_api_initialize(0, &test_services);

    status = sai_api_query(SAI_API_LAG, (void**) &lag_api);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[UT] Failed to query LAG API, status=%d\n", status);
        return 1;
    }

    printf("\n");
    printf("UT 1/6> Running...\n");
    test_create_and_remove_simple_lag(lag_api);
    printf("UT 1/6> Passed!\n\n");

    printf("UT 2/6> Running...\n");
    test_create_lag_members_and_link_to_lag(lag_api, port_list);
    printf("UT 2/6> Passed!\n\n");

    printf("UT 3/6> Running...\n");
    test_try_remove_lag_with_active_lag_members(lag_api, port_list);
    printf("UT 3/6> Passed!\n\n");

    printf("UT 4/6> Running...\n");
    test_create_lag_and_get_expected_attr_values(lag_api, port_list);
    printf("UT 4/6> Passed!\n\n");

    printf("UT 5/6> Running...\n");
    test_create_lag_member_and_get_expected_attr_values(lag_api, port_list);
    printf("UT 5/6> Passed!\n\n");

    printf("UT 6/6> Running...\n");
    test_try_create_lag_member_without_mandatory_attrs(lag_api, port_list);
    printf("UT 6/6> Passed!\n\n");

    status = sai_api_uninitialize();

    return 0;
}
