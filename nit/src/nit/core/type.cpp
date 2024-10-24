﻿#include "type.h"

namespace Nit
{
    TypeRegistry* type_registry = nullptr;

    void SetTypeRegistryInstance(TypeRegistry* type_registry_instance)
    {
        NIT_CHECK(type_registry_instance);
        type_registry = type_registry_instance;
    }

    TypeRegistry* GetTypeRegistryInstance()
    {
        NIT_CHECK(type_registry);
        return type_registry;
    }
    
    void SetRawData(const Type* type, void* array, u32 index, void* data)
    {
        NIT_CHECK(type && type->fn_set_data && array);
        type->fn_set_data(array, index, data);
    }

    void* GetRawData(const Type* type, void* array, u32 index)
    {
        NIT_CHECK(type && type->fn_get_data && array);
        return type->fn_get_data(array, index);
    }

    void LoadRawData(const Type* type, void* data)
    {
        NIT_CHECK(type);
        if (type->fn_invoke_load)
        {
            NIT_CHECK(data);
            type->fn_invoke_load(data);    
        }
    }

    void FreeRawData(const Type* type, void* data)
    {
        NIT_CHECK(type);
        if (type->fn_invoke_free)
        {
            NIT_CHECK(data);
            type->fn_invoke_free(data);    
        }
    }

    void SerializeRawData(const Type* type, void* data, YAML::Emitter& emitter)
    {
        NIT_CHECK(type);
        if (type->fn_invoke_serialize)
        {
            NIT_CHECK(data);
            type->fn_invoke_serialize(data, emitter);
        }
    }

    void DeserializeRawData(const Type* type, void* data, const YAML::Node& node)
    {
        NIT_CHECK(type);
        if (type->fn_invoke_deserialize)
        {
            NIT_CHECK(data);
            type->fn_invoke_deserialize(data, node);
        }
    }

    void InitTypeRegistry(u32 max_types)
    {
        NIT_CHECK(type_registry && !type_registry->types && !type_registry->enum_types);
        type_registry->types          = new Type[max_types];
        type_registry->max            = max_types;
        type_registry->count          = 0;
        type_registry->enum_types     = new EnumType[max_types];
        type_registry->max_enum_types = max_types;
        type_registry->enum_count     = 0;
    }

    bool IsEnumTypeRegistered(u64 type_hash)
    {
        NIT_CHECK(type_registry);
        return type_registry->hash_to_enum_index.count(type_hash) != 0;
    }

    EnumType* GetEnumType(u64 type_hash)
    {
        NIT_CHECK(type_registry && type_registry->enum_types);
        return &type_registry->enum_types[type_registry->hash_to_enum_index.at(type_hash)];
    }

    bool IsTypeRegistered(u64 type_hash)
    {
        NIT_CHECK(type_registry);
        return type_registry->hash_to_index.count(type_hash) != 0;
    }

    bool IsTypeRegistered(const String& name)
    {
        NIT_CHECK(type_registry && type_registry->types);
        return type_registry->name_to_index.count(name) != 0;
    }

    Type* GetType(u64 type_hash)
    {
        NIT_CHECK(type_registry && type_registry->types);
        return &type_registry->types[type_registry->hash_to_index.at(type_hash)];
    }

    Type* GetType(const String& name)
    {
        NIT_CHECK(type_registry && type_registry->types);
        return &type_registry->types[type_registry->name_to_index.at(name)];
    }
}
