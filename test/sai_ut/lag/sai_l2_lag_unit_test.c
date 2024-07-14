#include <stdio.h>
#include "sai.h"


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

int main()
{
    sai_status_t              status;
    sai_lag_api_t             *lag_api;
    sai_object_id_t           lag_oid1;
    sai_object_id_t           lag_member_id1;
    sai_object_id_t           lag_member_id2;
    sai_object_id_t           lag_oid2;
    sai_object_id_t           lag_member_id3;
    sai_object_id_t           lag_member_id4;

    status = sai_api_initialize(0, &test_services);

    status = sai_api_query(SAI_API_LAG, (void**)&lag_api);
    if (status != SAI_STATUS_SUCCESS) {
        printf("Failed to query LAG API, status=%d\n", status);
        return 1;
    }

    // Create the first LAG with two members 1 and 2
    lag_api->create_lag(&lag_oid1, 0, NULL);
    lag_api->create_lag_member(&lag_member_id1, 0, NULL);
    lag_api->create_lag_member(&lag_member_id2, 0, NULL);

    // Create the second LAG with two members 3 and 4
    lag_api->create_lag(&lag_oid2, 0, NULL);
    lag_api->create_lag_member(&lag_member_id3, 0, NULL);
    lag_api->create_lag_member(&lag_member_id4, 0, NULL);

    // Get the LAGs attributes
    lag_api->get_lag_attribute(lag_oid1, 0, NULL);
    lag_api->get_lag_attribute(lag_oid2, 0, NULL);

    // Get the attributes of the LAG members 1 and 3
    lag_api->get_lag_member_attribute(lag_member_id1, 0, NULL);
    lag_api->get_lag_member_attribute(lag_member_id3, 0, NULL);

    // Remove the LAG member 2 and verify it getting LAG1 Attrs
    lag_api->remove_lag_member(lag_member_id2);
    lag_api->get_lag_attribute(lag_oid1, 0, NULL);

    // Remove the LAG member 3 and verify it getting LAG2 Attrs
    lag_api->remove_lag_member(lag_member_id3);
    lag_api->get_lag_attribute(lag_oid2, 0, NULL);

    // Remove the LAG members 1 and 4
    lag_api->remove_lag_member(lag_member_id1);
    lag_api->remove_lag_member(lag_member_id4);

    // Remove the first and the second LAGs
    lag_api->remove_lag(lag_oid1);
    lag_api->remove_lag(lag_oid2);

    status = sai_api_uninitialize();

    printf("PASSED!\n");

    return 0;
}
