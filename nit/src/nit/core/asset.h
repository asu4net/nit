﻿#pragma once

namespace Nit
{
    struct AssetInfo
    {
        String type_name;
        String name;
        String path;
        ID     id              = 0;
        u32    version         = 0;
        bool   loaded          = false;
        u32    reference_count = 0;
    };
    
    template<typename T>
    struct AssetTypeArgs
    {
        FnLoad<T>        fn_load        = nullptr;
        FnFree<T>        fn_free        = nullptr;
        FnSerialize<T>   fn_serialize   = nullptr;
        FnDeserialize<T> fn_deserialize = nullptr;
        u32              max_elements   = DEFAULT_POOL_ELEMENT_COUNT;
    };

    struct AssetCreatedArgs
    {
        ID    id    = 0;
        Type* type  = nullptr;
    };
    
    struct AssetDestroyedArgs
    {
        ID    id    = 0;
        Type* type  = nullptr;
    };

    using AssetCreatedEvent = Event<const AssetCreatedArgs&>;
    using AssetRemovedEvent = Event<const AssetDestroyedArgs&>;
    
    struct AssetRegistry
    {
        Map<u64, Pool>          pools;
        Map<u64, u32>           hash_to_version;
        Map<ID, AssetInfo>      id_to_info;
        String                  extension = ".nit";
        AssetCreatedEvent       asset_created_event;
        AssetRemovedEvent       asset_destroyed_event;
    };
    
    void SetAssetRegistryInstance(AssetRegistry* asset_registry_instance);
    
    AssetRegistry* GetAssetRegistryInstance();
    
    template<typename T>
    void RegisterAssetType(const AssetTypeArgs<T>& args, u32 version = 0)
    {
        AssetRegistry* asset_registry = GetAssetRegistryInstance();
        u64 type_hash = GetTypeHash<T>();
        asset_registry->pools.insert({type_hash, {}});
        asset_registry->hash_to_version.insert({type_hash, version});
        
        if (!IsTypeRegistered<T>())
        {
            RegisterType<T>();
        }
        
        Type* type = GetType<T>();
        SetInvokeLoadFunction(type, args.fn_load);
        SetInvokeFreeFunction(type, args.fn_free);
        SetSerializeFunction(type, args.fn_serialize);
        SetDeserializeFunction(type, args.fn_deserialize);
        
        InitPool<T>(&asset_registry->pools.at(type_hash), args.max_elements);
    }

    bool IsAssetTypeRegistered(u64 type_hash);
    
    template<typename T>
    bool IsAssetTypeRegistered()
    {
        return IsAssetTypeRegistered(GetTypeHash<T>());
    }
    
    void BuildAssetPath(const String& name, String& path);
    
    void PushAssetInfo(AssetInfo& asset_info, bool build_path);
    
    void EraseAssetInfo(ID id);
    
    ID DeserializeAssetFromString(const String& asset_str);
    
    ID DeserializeAssetFromFile(const String& file_path);
    
    void SerializeAssetToString(ID id, String& result);
    
    void SerializeAssetToFile(ID id);
    
    void InitAssetRegistry();
    
    Pool& GetAssetPool(u64 type_hash);
    
    Pool& GetAssetPool(const String& type_name);

    u32 GetLastAssetVersion(u64 type_hash);
    
    u32 GetLastAssetVersion(const String& type_name);

    template<typename T>
    u32 GetLastAssetVersion()
    {
        return GetLastAssetVersion(GetTypeHash<T>());
    }
    
    template<typename T>
    Pool& GetAssetPool()
    {
        return GetAssetPool(GetTypeHash<T>());
    }

    void FindAssetsByName(const String& name, Array<ID>& asset_ids);
    
    ID FindAssetByName(const String& name);
    
    template<typename T>
    T& GetAssetData(ID id)
    {
        Pool& pool = GetAssetPool<T>();
        return GetPoolElement<T>(&pool, id);
    }

    template<typename T>
    T* GetAssetDataPtr(ID id)
    {
        Pool& pool = GetAssetPool<T>();
        return GetPoolElementPtr<T>(&pool, id);
    }

    bool IsAssetValid(ID id);

    bool IsAssetLoaded(ID id);
    
    template<typename T>
    ID CreateAsset(const String& name, const String& path = "", const T& data = {})
    {
        Pool& pool = GetAssetPool<T>();
        ID id; InsertPoolElement(&pool, id, data);
        AssetInfo info { pool.type->name, name, path, id, GetLastAssetVersion<T>(), };
        PushAssetInfo(info, true);
        AssetCreatedArgs args;
        args.id   = id;
        args.type = pool.type;
        Broadcast<const AssetCreatedArgs&>(GetAssetRegistryInstance()->asset_created_event, args);
        return id;
    }
    
    void LoadAsset(ID id, bool force_reload = false);

    void FreeAsset(ID id);

    void RetainAsset(ID id);
    
    void ReleaseAsset(ID id, bool force_free = false);
    
    void DestroyAsset(ID id);
}
