#include "sai.h"
#include "stub_sai.h"


// Debug options
// #define DEBUG


// ------------------------------ LAG Database -----------------------------

// Limits
#define MAX_NUMBER_OF_LAGS 5
#define MAX_NUMBER_OF_LAG_MEMBERS 16

// LAG_MEMBER Entry
typedef struct _lag_member_db_entry_t {
    sai_object_id_t oid;
    sai_object_id_t port_oid;
    sai_object_id_t lag_oid;
    bool            is_used;

} lag_member_db_entry_t;

// LAG Entry
typedef struct _lag_db_entry_t {
    sai_object_id_t oid;
    sai_object_id_t members_ids[MAX_NUMBER_OF_LAG_MEMBERS];
    bool            is_used;

} lag_db_entry_t;

// Database
struct lag_db_t {
    lag_db_entry_t        lags[MAX_NUMBER_OF_LAGS];
    lag_member_db_entry_t members[MAX_NUMBER_OF_LAG_MEMBERS];

} lag_db;

// ------------------------- LAG DB Helper functions -----------------------

int lag_db_find_new_object_slot(const int arr_size, sai_object_type_t otype)
{
    // Goes through the objects and looks for the unused one
    for (int i = 0; i < arr_size; ++i)
    {
        bool is_used;
        switch (otype)
        {
        case SAI_OBJECT_TYPE_LAG:
            is_used = lag_db.lags[i].is_used;
            break;

        case SAI_OBJECT_TYPE_LAG_MEMBER:
            is_used = lag_db.members[i].is_used;
            break;

        default:
            printf("[ERROR] <LAG DB> Unknown object type: %i", (int) otype);
            return -1;
        }

        if (!is_used)
        {
            return i;
        }
    }

    printf("[ERROR] <LAG DB> DB with objects of type %i is full!\n", (int) otype);
    return -1;
}

sai_status_t lag_db_lag_has_active_members(sai_object_id_t lag_oid, bool *has_active_members)
{
    // Get LAG index in the DB
    uint32_t lag_db_index;
    sai_status_t status = stub_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] <LAG DB> Cannot find LAG with ID: 0x%lX!\n", lag_oid);
        return status;
    }

    *has_active_members = false;
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        sai_object_id_t member_oid = lag_db.lags[lag_db_index].members_ids[i];
        if (member_oid != (sai_object_id_t) 0)
        {
            *has_active_members = true;
            printf("[ERROR] <LAG DB> LAG 0x%lX has active member " \
                   "with OID: 0x%lX\n", lag_oid, member_oid);
            break;
        }
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t lag_db_link_member_to_lag(sai_object_id_t lag_oid,
                                       sai_object_id_t member_oid)
{
    // Get LAG index in the DB
    uint32_t lag_db_index;
    sai_status_t status = stub_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] <LAG DB> Cannot find LAG with ID: 0x%lX!\n", lag_oid);
        return status;
    }

    // Checks if the lag member has already been linked to the lag
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        sai_object_id_t member_db_oid = lag_db.lags[lag_db_index].members_ids[i];
        if (member_db_oid == member_oid)
        {
            printf("[WARNING] <LAG DB> LAG_MEMBER 0x%lX has already " \
                   "been linked to the LAG 0x%lX\n", member_oid, lag_oid);
            return SAI_STATUS_SUCCESS;
        }
    }

    // Goes through the the lag members and looks for the unused one to link
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        sai_object_id_t *member_db_oid = &lag_db.lags[lag_db_index].members_ids[i];
        if (*member_db_oid == (sai_object_id_t) 0)
        {
            *member_db_oid = member_oid;
            printf("[INFO] LAG_MEMBER 0x%lX has been " \
                   "linked to LAG 0x%lX\n",  member_oid, lag_oid);
            return SAI_STATUS_SUCCESS;
        }
    }

    printf("[ERROR] <LAG DB> Cannot link LAG_MEMBER 0x%lX to LAG 0x%lX, " \
           "link list is FULL!\n", member_oid, lag_oid);
    return SAI_STATUS_FAILURE;
}

sai_status_t lag_db_unlink_member_from_lag(sai_object_id_t lag_oid,
                                           sai_object_id_t member_oid)
{
    // Get LAG index in the DB
    uint32_t lag_db_index;
    sai_status_t status = stub_object_to_type(lag_oid, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] <LAG DB> Cannot find LAG with ID: 0x%lX!\n", lag_oid);
        return status;
    }

    // Goes through the lag member oids and removes matching one
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        sai_object_id_t *member_db_oid = &lag_db.lags[lag_db_index].members_ids[i];
        if (*member_db_oid == member_oid)
        {
            *member_db_oid = (sai_object_id_t) 0;
            printf("[INFO] LAG_MEMBER 0x%lX has been " \
                   "unlinked from LAG 0x%lX\n",  member_oid, lag_oid);
            return SAI_STATUS_SUCCESS;
        }
    }

    printf("[ERROR] <LAG DB> Cannot unlink LAG_MEMBER 0x%lX from LAG 0x%lX, " \
           "the member is not linked!\n", member_oid, lag_oid);
    return SAI_STATUS_FAILURE;
}

void lag_db_dbg_print_db()
{
    printf("--------------------------------------------------------\n");
    printf("db: LAGs: \n");

    for (int i = 0; i < MAX_NUMBER_OF_LAGS; ++i)
    {
        if (!lag_db.lags[i].is_used) continue;
        printf("db:   #%i oid: 0x%lX", i, lag_db.lags[i].oid);
        printf("\t( members: ");
        for (int j = 0; j < MAX_NUMBER_OF_LAG_MEMBERS; ++j)
        {
            sai_object_id_t m_oid = lag_db.lags[i].members_ids[j];
            if (m_oid == (sai_object_id_t) 0) continue;
            printf("0x%lX ", m_oid);
        }
        printf(")\n");
    }

    printf("db: LAG_MEMBERs: \n");
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        if (!lag_db.members[i].is_used) continue;
        printf("db:   #%i oid: 0x%lX", i, lag_db.members[i].oid);
        printf("\t( port: 0x%lX, lag: 0x%lX )\n", lag_db.members[i].port_oid,
                                                  lag_db.members[i].lag_oid);
    }
    printf("--------------------------------------------------------\n");
}

// -------------------------------------------------------------------------

// --------------------------- LAG Attr Interface --------------------------

static const sai_attribute_entry_t lag_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST, false, false, false, true,
      "List of ports in LAG", SAI_ATTR_VAL_TYPE_OIDLIST },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

static const sai_attribute_entry_t lag_member_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID, true, true, false, true,
      "LAG ID", SAI_ATTR_VAL_TYPE_OID },
    { SAI_LAG_MEMBER_ATTR_PORT_ID, true, true, false, true,
      "PORT ID", SAI_ATTR_VAL_TYPE_OID },
    { END_FUNCTIONALITY_ATTRIBS_ID, false, false, false, false,
      "", SAI_ATTR_VAL_TYPE_UNDETERMINED }
};

sai_status_t _get_lag_port_list(sai_object_id_t lag_id, sai_object_list_t* obj_list)
{
    sai_status_t status;
    uint32_t     lag_db_index;

    // Get LAG index in the DB
    status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("[ERROR] Cannot get LAG 0x%lX DB index!\n", lag_id);
        return status;
    }

    // Count PORT_LIST size
    int port_list_counter = 0;
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        if (lag_db.lags[lag_db_index].members_ids[i] != (sai_object_id_t) 0)
        {
            port_list_counter++;
        }
    }

    // Allocate memory for PORT_LIST
    sai_object_id_t *port_list = (sai_object_id_t*) malloc(port_list_counter * sizeof(sai_object_id_t));
    int port_list_write_pos = 0;

    // Write ports into PORT_LIST
    for (int i = 0; i < MAX_NUMBER_OF_LAG_MEMBERS; ++i)
    {
        sai_object_id_t member_ids_oid = lag_db.lags[lag_db_index].members_ids[i];
        if (member_ids_oid == (sai_object_id_t) 0)
        {
            continue;
        }

        uint32_t member_db_index;
        status = stub_object_to_type(member_ids_oid, SAI_OBJECT_TYPE_LAG_MEMBER, &member_db_index);
        if (status != SAI_STATUS_SUCCESS)
        {        
            printf("[ERROR] Cannot get LAG_MEMBER 0x%lX DB index!\n", member_ids_oid);
            return status;
        }

        port_list[port_list_write_pos++] = lag_db.members[member_db_index].port_oid;
    }

    obj_list->list = port_list;
    obj_list->count = port_list_counter;

    return SAI_STATUS_SUCCESS;
}

sai_status_t get_lag_attribute(_In_ const sai_object_key_t   *key,
                               _Inout_ sai_attribute_value_t *value,
                               _In_ uint32_t                  attr_index,
                               _Inout_ vendor_cache_t        *cache,
                               void                          *arg)
{
    sai_attr_id_t attr_id = *(sai_attr_id_t*)arg;
    assert((SAI_LAG_ATTR_PORT_LIST == attr_id));

    switch (attr_id) {
    case SAI_LAG_ATTR_PORT_LIST:
        _get_lag_port_list(key->object_id, &value->objlist);
        break;

    default:
        printf("[ERROR] Got unexpected attribute ID!\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

sai_status_t get_lag_member_attribute(_In_ const sai_object_key_t   *key,
                                      _Inout_ sai_attribute_value_t *value,
                                      _In_ uint32_t                  attr_index,
                                      _Inout_ vendor_cache_t        *cache,
                                      void                          *arg)
{
    sai_status_t status;
    uint32_t     db_index;
    
    sai_attr_id_t attr_id = *(sai_attr_id_t*)arg;
    assert((SAI_LAG_MEMBER_ATTR_LAG_ID  == attr_id) ||
           (SAI_LAG_MEMBER_ATTR_PORT_ID == attr_id));

    status = stub_object_to_type(key->object_id, SAI_OBJECT_TYPE_LAG_MEMBER, &db_index);
    if (status != SAI_STATUS_SUCCESS)
    {        
        printf("[ERROR] Cannot get LAG DB index.\n");
        return status;
    }

    switch (attr_id)
    {
    case SAI_LAG_MEMBER_ATTR_LAG_ID:
        value->oid = lag_db.members[db_index].lag_oid;
        break;

    case SAI_LAG_MEMBER_ATTR_PORT_ID:
        value->oid = lag_db.members[db_index].port_oid;
        break;

    default:
        printf("[ERROR] Got unexpected attribute ID\n");
        return SAI_STATUS_FAILURE;
    }

    return SAI_STATUS_SUCCESS;
}

static const sai_vendor_attribute_entry_t lag_vendor_attribs[] = {
    { SAI_LAG_ATTR_PORT_LIST,
      { false, false, false, true },
      { false, false, false, true },
      get_lag_attribute, (void*) SAI_LAG_ATTR_PORT_LIST,
      NULL, NULL }
};

static const sai_vendor_attribute_entry_t lag_member_vendor_attribs[] = {
    { SAI_LAG_MEMBER_ATTR_LAG_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_LAG_ID,
      NULL, NULL },
    { SAI_LAG_MEMBER_ATTR_PORT_ID,
      { true, false, false, true },
      { true, false, false, true },
      get_lag_member_attribute, (void*) SAI_LAG_MEMBER_ATTR_PORT_ID,
      NULL, NULL }
};

// -------------------------------------------------------------------------

// ------------------------------ LAG Interface ----------------------------

sai_status_t stub_create_lag(
    _Out_ sai_object_id_t* lag_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t    status;
    char            list_str[MAX_LIST_VALUE_STR_LEN];

    // Get formatted string from attributes
    sai_attr_list_to_str(attr_count, attr_list, lag_attribs,
                         MAX_LIST_VALUE_STR_LEN, list_str);

    // Check mendatory attributes
    status = check_attribs_metadata(attr_count, attr_list, lag_attribs,
                                    lag_vendor_attribs, SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("[ERROR] Invalid LAG attributes on CREATE!\n");
        return status;
    }

    // Find index in the DB where lag can be registered
    int lag_db_index = lag_db_find_new_object_slot(MAX_NUMBER_OF_LAGS,
                                                   SAI_OBJECT_TYPE_LAG);
    
    if (lag_db_index == -1)
    {
        printf("[ERROR] Cannot create LAG 0x%lX, the LAG DB is FULL!\n", *lag_id);
        return SAI_STATUS_FAILURE;
    }

    // Create an object
    status = stub_create_object(SAI_OBJECT_TYPE_LAG, (uint32_t) lag_db_index, lag_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] Cannot create a LAG OID!\n");
        return status;
    }

    // Register lag in the DB
    lag_db.lags[lag_db_index].is_used = true;
    lag_db.lags[lag_db_index].oid = *lag_id;

    printf("[INFO] Create LAG: 0x%lX (%s)\n", *lag_id, list_str);

#ifdef DEBUG
    lag_db_dbg_print_db();
#endif

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag(
    _In_ sai_object_id_t  lag_id)
{
    sai_status_t status;
    uint32_t     lag_db_index;

    // Get lag index in the DB
    status = stub_object_to_type(lag_id, SAI_OBJECT_TYPE_LAG, &lag_db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] Cannot get LAG db id!\n");
        return status;
    }

    // Check for active members
    bool has_active_members;
    status = lag_db_lag_has_active_members(lag_id, &has_active_members);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("[ERROR] Failed to check the LAG 0x%lX for active members!", lag_id);
        return SAI_STATUS_FAILURE;
    }

    if (has_active_members)
    {
        return SAI_STATUS_FAILURE;
    }

    // Remove lag from the DB
    lag_db.lags[lag_db_index].is_used = false;
    lag_db.lags[lag_db_index].oid = (sai_object_id_t) 0;
    memset(lag_db.lags[lag_db_index].members_ids, 0,
           sizeof(lag_db.lags[lag_db_index].members_ids));

    printf("[INFO] Remove LAG: 0x%lX\n", lag_id);

#ifdef DEBUG
    lag_db_dbg_print_db();
#endif

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_attribute(
    _In_ sai_object_id_t  lag_id,
    _In_ const sai_attribute_t *attr)
{
    // Not implemented
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_attribute(
    _In_ sai_object_id_t lag_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    for (uint32_t i = 0; i < attr_count; ++i)
    {
        get_lag_attribute((sai_object_key_t*) &lag_id,
                          &attr_list[i].value, 0, NULL,
                          &attr_list[i].id);
    }
    printf("[INFO] Get LAG Attrs: 0x%lX\n", lag_id);
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_create_lag_member(
    _Out_ sai_object_id_t* lag_member_id,
    _In_ uint32_t attr_count,
    _In_ sai_attribute_t *attr_list)
{
    sai_status_t                    status;
    const sai_attribute_value_t     *port_oid, *lag_oid;
    uint32_t                        port_id_index, lag_id_index;
    char                            list_str[MAX_LIST_VALUE_STR_LEN];

    // Check mendatory attributes
    status = check_attribs_metadata(attr_count, attr_list,
                                    lag_member_attribs,
                                    lag_member_vendor_attribs,
                                    SAI_OPERATION_CREATE);
    if (status != SAI_STATUS_SUCCESS)
    {
        printf("[ERROR] Invalid LAG_MEMBER attributes on CREATE!\n");
        return status;
    }

    // Get formatted string from attributes
    sai_attr_list_to_str(attr_count, attr_list, lag_member_attribs,
                         MAX_LIST_VALUE_STR_LEN, list_str);

    // Look for the index where lag member can be registered
    uint32_t lag_member_db_index = lag_db_find_new_object_slot(MAX_NUMBER_OF_LAG_MEMBERS,
                                                               SAI_OBJECT_TYPE_LAG_MEMBER);

    // Create lag member object
    status = stub_create_object(SAI_OBJECT_TYPE_LAG_MEMBER, lag_member_db_index, lag_member_id);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] Cannot create a LAG_MEMBER OID!\n");
        return status;
    }

    // Read port id
    status = find_attrib_in_list(attr_count, attr_list,
                                 SAI_LAG_MEMBER_ATTR_PORT_ID,
                                 &port_oid, &port_id_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] Cannot find attrib: SAI_LAG_MEMBER_ATTR_PORT_ID\n");
        return status;
    }

    // Read lag id
    status = find_attrib_in_list(attr_count, attr_list,
                                 SAI_LAG_MEMBER_ATTR_LAG_ID,
                                 &lag_oid, &lag_id_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] Cannot find attrib: SAI_LAG_MEMBER_ATTR_LAG_ID\n");
        return status;
    }

    // Register lag member in the db
    lag_db.members[lag_member_db_index].oid = *lag_member_id;
    lag_db.members[lag_member_db_index].port_oid = port_oid->u64;
    lag_db.members[lag_member_db_index].lag_oid = lag_oid->u64;
    lag_db.members[lag_member_db_index].is_used = true;

    // Link member to LAG
    lag_db_link_member_to_lag(lag_oid->u64, *lag_member_id);

    printf("[INFO] Create LAG_MEMBER: 0x%lX (%s)\n", *lag_member_id, list_str);

#ifdef DEBUG
    lag_db_dbg_print_db();
#endif

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_remove_lag_member(
    _In_ sai_object_id_t  lag_member_id)
{
    sai_status_t status;
    uint32_t     lag_member_db_index;

    // Get lag_MEMBER index in the DB
    status = stub_object_to_type(lag_member_id, SAI_OBJECT_TYPE_LAG_MEMBER, &lag_member_db_index);
    if (status != SAI_STATUS_SUCCESS) {
        printf("[ERROR] Cannot get LAG_MEMBER db id!\n");
        return status;
    }

    lag_db_unlink_member_from_lag(lag_db.members[lag_member_db_index].lag_oid, lag_member_id);

    // Remove lag from the DB
    lag_db.members[lag_member_db_index].oid = (sai_object_id_t) 0;
    lag_db.members[lag_member_db_index].port_oid = (sai_object_id_t) 0;
    lag_db.members[lag_member_db_index].lag_oid = (sai_object_id_t) 0;
    lag_db.members[lag_member_db_index].is_used = false;

    printf("[INFO] Remove LAG_MEMBER: 0x%lX\n", lag_member_id);

#ifdef DEBUG
    lag_db_dbg_print_db();
#endif

    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_set_lag_member_attribute(
    _In_ sai_object_id_t  lag_member_id,
    _In_ const sai_attribute_t *attr)
{
    // Not implemented
    return SAI_STATUS_SUCCESS;
}

sai_status_t stub_get_lag_member_attribute(
    _In_ sai_object_id_t lag_member_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list)
{
    for (uint32_t i = 0; i < attr_count; ++i)
    {
        get_lag_member_attribute((sai_object_key_t*) &lag_member_id,
                                 &attr_list[i].value, 0, NULL,
                                 &attr_list[i].id);
    }

    printf("[INFO] Get LAG_MEMBER Attrs: 0x%lX\n", lag_member_id);
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

// -------------------------------------------------------------------------
