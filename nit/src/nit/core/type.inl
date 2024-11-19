﻿#pragma once

namespace nit
{
    template<typename T>
    void set_invoke_load_function(Type* type, FnLoad<T> fn_load)
    {
        NIT_CHECK(type);
        if (fn_load && !type->fn_invoke_load)
        {
            type->fn_invoke_load = [fn_load] (void* data) {
                T* casted_data = static_cast<T*>(data);
                fn_load(casted_data);
            };
        }
    }

    template<typename T>
    void set_invoke_free_function(Type* type, FnFree<T> fn_free)
    {
        NIT_CHECK(type);
        if (fn_free && !type->fn_invoke_free)
        {
            type->fn_invoke_free = [fn_free] (void* data) {
                T* casted_data = static_cast<T*>(data);
                fn_free(casted_data);
            };
        }
    }

    template<typename T>
    void set_serialize_function(Type* type, FnSerialize<T> fn_serialize)
    {
        NIT_CHECK(type);
        
        if (fn_serialize)
        {
            type->fn_invoke_serialize = [fn_serialize] (void* data, YAML::Emitter& emitter) {
                const T* casted_data = static_cast<const T*>(data);
                fn_serialize(casted_data, emitter);
            };
        }
    }

    template<typename T>
    void set_deserialize_function(Type* type, FnDeserialize<T> fn_deserialize)
    {
        NIT_CHECK(type);
        
        if (fn_deserialize)
        {
            type->fn_invoke_deserialize = [fn_deserialize] (void* data, const YAML::Node& node) {
                T* casted_data = static_cast<T*>(data);
                fn_deserialize(casted_data, node);
            };
        }
    }

#ifdef NIT_EDITOR_ENABLED
    template<typename T>
    void set_draw_editor_function(Type* type, FnDrawEditor<T> fn_draw_editor)
    {
        NIT_CHECK(type);
        
        if (fn_draw_editor)
        {
            type->fn_invoke_draw_editor = [fn_draw_editor] (void* data) {
                T* casted_data = static_cast<T*>(data);
                fn_draw_editor(casted_data);
            };
        }
    }
#endif

    template<typename T>
    u64 get_type_hash()
    {
        return typeid(T).hash_code();
    }
    
    template<typename T>
    void init_type(Type& type, const TypeArgs<T>& args)
    {
        type.hash = get_type_hash<T>();
        
        static const String STRUCT_TEXT = "struct "; 
        static const String CLASS_TEXT  = "class "; 
        static const String NAMESPACE_TEXT  = "nit::"; 
        type.name = typeid(T).name();
        Replace(type.name, STRUCT_TEXT, "");
        Replace(type.name, CLASS_TEXT , "");
        Replace(type.name, NAMESPACE_TEXT , "");
        
        type.fn_set_data = [](void* elements, u32 element_index, void* data) {

            static T default_data;
            T* casted_data = data != nullptr ? static_cast<T*>(data) : nullptr;
            T* casted_elements = static_cast<T*>(elements);
            casted_elements[element_index] = casted_data ? *casted_data : default_data;
        };
        
        type.fn_get_data = [](void* elements, u32 element_index) -> void* {
            T* casted_elements = static_cast<T*>(elements);
            T* data = &casted_elements[element_index];
            return data;
        };

        type.fn_resize_data = [](void* elements, u32 max, u32 new_max) -> void* {
            T* casted_elements = static_cast<T*>(elements);
            T* new_elements = new T[new_max];
            std::copy_n(casted_elements, max, new_elements);
            delete [] new_elements;
            return new_elements;
        };
        
        if (auto fn_serialize = args.fn_serialize)
        {
            set_serialize_function(&type, fn_serialize);
        }

        if (auto fn_deserialize = args.fn_deserialize)
        {
            set_deserialize_function(&type, fn_deserialize);
        }

        if (auto fn_load = args.fn_load)
        {
            set_invoke_load_function(&type, fn_load);
        }

        if (auto fn_free = args.fn_free)
        {
            set_invoke_free_function(&type, fn_free);
        }

#ifdef NIT_EDITOR_ENABLED
        if (auto fn_draw_editor = args.fn_draw_editor)
        {
            set_draw_editor_function(&type, fn_draw_editor);
        }
#endif
    }

    template<typename T>
    T* SetArrayData(void* array, u32 index, const T& data)
    {
        T* casted_array = static_cast<T*>(array);
        T* saved_data = &casted_array[index];
        *saved_data = data;
        return saved_data;
    }

    template<typename T>
    T* GetArrayData(void* array, u32 index)
    {
        T* casted_array = static_cast<T*>(array);
        return &casted_array[index];
    }

    template <typename T>
    void InitEnumType(EnumType& enum_type)
    {
        enum_type.hash = typeid(T).hash_code();
        static const String ENUM_TEXT  = "enum class "; 
        static const String ENUM_CLASS_TEXT  = "enum "; 
        enum_type.name = typeid(T).name();
        Replace(enum_type.name, ENUM_TEXT , "");
        Replace(enum_type.name, ENUM_CLASS_TEXT , "");
    }

    template <typename T>
    void enum_register_value(EnumType* enum_type, const String& value_name, T value)
    {
        NIT_CHECK(enum_type);
        u8 index = (u8) value;
        enum_type->name_to_index.insert({ value_name, index });
        enum_type->index_to_name.insert({ index, value_name });
    }

    template <typename T>
    T enum_from_string(EnumType* enum_type, const String& value_name)
    {
        NIT_CHECK(enum_type && enum_type->name_to_index.count(value_name) != 0);
        return (T) enum_type->name_to_index.at(value_name);
    }

    template <typename T>
    String enum_to_string(EnumType* enum_type, T value)
    {
        u8 index = (u8) value;
        NIT_CHECK(enum_type && enum_type->index_to_name.count(index) != 0);
        return enum_type->index_to_name.at(index);
    }

    template <typename T>
    void type_register(const TypeArgs<T>& args)
    {
        TypeRegistry* type_registry = type_registry_get_instance();
        
        if (type_registry->count >= type_registry->max)
        {
            NIT_CHECK_MSG(false, "Max type count reached!");
            return;
        }
        Type& type = type_registry->types[type_registry->count]; 
        init_type(type, args);
        NIT_CHECK(type_registry->hash_to_index.count(type.hash) == 0);
        type_registry->hash_to_index[type.hash] = type_registry->count;
        NIT_CHECK(type_registry->name_to_index.count(type.name) == 0);
        type_registry->name_to_index[type.name] = type_registry->count;
        ++type_registry->count;
    }

    template <typename T>
    void enum_register()
    {
        TypeRegistry* type_registry = type_registry_get_instance();
        if (type_registry->enum_count >= type_registry->max_enum_types)
        {
            NIT_CHECK_MSG(false, "Max enum type count reached!");
            return;
        }
        
        EnumType& enum_type = type_registry->enum_types[type_registry->enum_count];
        InitEnumType<T>(enum_type);
        NIT_CHECK(type_registry->hash_to_index.count(enum_type.hash) == 0);
        type_registry->hash_to_enum_index[enum_type.hash] = type_registry->enum_count;
        ++type_registry->enum_count;
    }

    template <typename T>
    bool enum_registered()
    {
        return enum_registered(get_type_hash<T>());
    }

    template <typename T>
    EnumType* enum_type_get()
    {
        return enum_type_get(get_type_hash<T>());
    }

    template <typename EType, typename T>
    void enum_register_value(const String& value_name, T value)
    {
        EnumType* enum_type = enum_type_get<EType>();
        u8 index = (u8) value;
        enum_type->name_to_index.insert({ value_name, index });
        enum_type->index_to_name.insert({ index, value_name });
    }
    
    template <typename T>
    T enum_from_string(const String& value_name)
    {
        return enum_from_string<T>(enum_type_get<T>(), value_name);
    }
    
    template <typename EType, typename T>
    String enum_to_string(T value)
    {
        return enum_to_string(enum_type_get<EType>(), value);
    }

    template <typename T>
    bool type_exists()
    {
        return type_exists(get_type_hash<T>());
    }

    template <typename T>
    Type* type_get()
    {
        return type_get(get_type_hash<T>());
    }
}