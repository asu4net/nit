#pragma once

namespace nit
{
    template<typename T>
    void asset_register_type(const AssetTypeArgs<T>& args, u32 version)
    {
        AssetRegistry* asset_registry = asset_get_instance();
        AssetPool* asset_pool = &asset_registry->asset_pools.emplace_back();
        
        if (!type_exists<T>())
        {
            type_register<T>();
        }
        
        Type* type = type_get<T>();
        
        set_invoke_load_function  (type,  args.fn_load);
        set_invoke_free_function  (type,  args.fn_free);
        set_serialize_function   (type,  args.fn_serialize);
        set_deserialize_function (type,  args.fn_deserialize);
#ifdef NIT_EDITOR_ENABLED
        set_draw_editor_function  (type,  args.fn_draw_editor);
#endif
        pool_load<T>(&asset_pool->data_pool, args.max_elements);
        asset_pool->asset_infos = new AssetInfo[args.max_elements];
    }
    
    template<typename T>
    AssetPool* asset_get_pool()
    {
        return asset_get_pool(type_get<T>());
    }
    
    template<typename T>
    bool asset_type_registered()
    {
        return asset_type_registered(type_get<T>());
    }
    
    template<typename T>
    u32 asset_get_last_version()
    {
        return asset_get_last_version(type_get<T>());
    }
    
    template<typename T>
    T* asset_get_data(const AssetHandle& asset)
    {
        AssetPool* pool = asset_get_pool_safe(type_get<T>());
        return pool_get_data<T>(&pool->data_pool, asset.data_id);
    }
    
    template<typename T>
    AssetHandle asset_create(const String& name, const String& path, const T& data)
    {
        AssetPool* pool = asset_get_pool_safe(type_get<T>());
        Pool* data_pool = &pool->data_pool;
        Type* type = data_pool->type;
        u32 data_id; pool_insert_data(data_pool, data_id, data);
        UUID asset_id = uuid_generate();
        asset_get_instance()->id_to_data_id.insert({asset_id, data_id});
        AssetInfo info{type, name, path, asset_id, asset_get_last_version<T>(), false, 0, data_id };
        asset_push_info(info,  pool_index_of(data_pool, data_id), true);
        AssetHandle asset_handle = asset_create_handle(&info);
        event_broadcast<const AssetCreatedArgs&>(asset_get_instance()->asset_created_event, {asset_handle});
        return asset_handle;
    }
}

template<>
struct YAML::convert<nit::AssetHandle>
{
    static Node encode(const nit::AssetHandle& h)
    {
        Node node;
        node.push_back(h.name);
        node.push_back(h.type->name);
        node.push_back((u64) h.id);
        node.SetStyle(EmitterStyle::Flow);
        return node;
    }
    
    static bool decode(const Node& node, nit::AssetHandle& h)
    {
        if (!node.IsSequence() || node.size() != 3)
            return false;

        h.name    = node[0].as<String>();
        h.type    = nit::type_get(node[1].as<String>());
        h.id      = (nit::UUID) node[2].as<u64>();
        return true;
    }
};

inline YAML::Emitter& operator<<(YAML::Emitter& out, const nit::AssetHandle& h)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << h.name << (h.type ? h.type->name : "") << (u64) h.id << YAML::EndSeq;
    return out;
}