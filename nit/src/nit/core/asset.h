#pragma once

namespace nit
{
    struct AssetInfo
    {
        Type*  type;
        String name;
        String path;
        UUID   id              = {};
        u32    version         = 0;
        bool   loaded          = false;
        u32    reference_count = 0;
        u32    data_id         = SparseSet::INVALID;
    };
    
    template<typename T>
    struct AssetTypeArgs
    {
        FnLoad<T>        fn_load        = nullptr;
        FnFree<T>        fn_free        = nullptr;
        FnSerialize<T>   fn_serialize   = nullptr;
        FnDeserialize<T> fn_deserialize = nullptr;
#ifdef NIT_EDITOR_ENABLED
        FnDrawEditor<T>  fn_draw_editor = nullptr;
#endif
        u32              max_elements   = 100;
    };

    struct AssetHandle
    {
        String name;
        Type*  type    = nullptr;
        UUID   id;
        u32    data_id = SparseSet::INVALID;
    };
    
    struct AssetCreatedArgs
    {
        AssetHandle asset_handle;
    };
    
    struct AssetDestroyedArgs
    {
        AssetHandle asset_handle;
    };
    
    using AssetCreatedEvent      = Event<const AssetCreatedArgs&>;
    using AssetRemovedEvent      = Event<const AssetDestroyedArgs&>;
    using AssetCreatedListener   = Listener<const AssetCreatedArgs&>;
    using AssetDestroyedListener = Listener<const AssetDestroyedArgs&>;

    struct AssetPool
    {
        Pool         data_pool;
        u32          version       = 0;
        AssetInfo*   asset_infos   = nullptr;
    };
    
    struct AssetRegistry
    {
        Array<AssetPool>     asset_pools;
        String               extension = ".nit";
        AssetCreatedEvent    asset_created_event;
        AssetRemovedEvent    asset_destroyed_event;
        Map<UUID, u32>       id_to_data_id;
    };

    AssetHandle    asset_create_handle           (AssetInfo* asset_info);
    void           asset_retarget_handle         (AssetHandle& asset_handle);
    Path           asset_get_directory           ();
    void           asset_registry_set_instance            (AssetRegistry* asset_registry_instance);
    AssetRegistry* asset_get_instance            ();
    AssetPool*     asset_get_pool                (Type* type);
    AssetPool*     asset_get_pool_safe           (const AssetHandle& asset);
    AssetPool*     asset_get_pool_safe           (Type* type);
    AssetPool*     asset_get_pool_safe           (const AssetInfo& asset_info);
    bool           asset_type_registered         (Type* type);
    void           asset_build_path              (const String& name, String& path);
    void           asset_push_info               (AssetInfo& asset_info, u32 index, bool build_path);
    void           asset_erase_info              (AssetInfo& asset_info, SparseSetDeletion deletion);
    AssetInfo*     asset_get_info                (const AssetHandle& asset);
    AssetHandle    asset_deserialize_from_string (const String& asset_str);
    AssetHandle    asset_deserialize_from_file   (const String& file_path);
    void           asset_serialize_to_string     (AssetHandle& asset, String& result);
    void           asset_serialize_to_file       (AssetHandle& asset);
    void           asset_registry_deserialize();
    u32            asset_get_last_version        (Type* type);
    u32            asset_get_last_version        (const String& type_name);
    void           asset_find_by_name            (const String& name, Array<AssetHandle>& assets);
    AssetHandle    asset_find_by_name            (const String& name);
    void           asset_find_by_type            (Type* type, Array<AssetHandle>& assets);
    bool           asset_valid                   (const AssetHandle& asset);
    bool           asset_loaded                  (const AssetHandle& asset);
    AssetHandle    asset_create                  (Type* type, const String& name, const String& path, void* data = nullptr);
    void           asset_load                    (const AssetHandle& asset, bool force_reload = false);
    void           asset_retain                  (const AssetHandle& asset);
    void           asset_free                    (const AssetHandle& asset);
    void           asset_release                 (AssetHandle& asset, bool force_free = false);
    void           asset_destroy                 (AssetHandle& asset);
    
    template<typename T>   void        asset_register_type    (const AssetTypeArgs<T>& args, u32 version = 0);
    template<typename T>   AssetPool*  asset_get_pool         ();
    template<typename T>   bool        asset_type_registered  ();
    template<typename T>   u32         asset_get_last_version ();
    template<typename T>   T*          asset_get_data         (const AssetHandle& asset);
    template<typename T>   AssetHandle asset_create           (const String& name, const String& path = "", const T& data = {});
}

#include "asset.inl"