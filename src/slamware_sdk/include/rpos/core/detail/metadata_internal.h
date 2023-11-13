#pragma once

#include <rpos/core/metadata.h>
#include <rpos/system/serialization/json_serialization.h>

namespace rpos { namespace core { 

    template< class T >
    void Metadata::set(const std::string& key, const T& value)
    {
        Json::FastWriter writer;
        writer.omitEndingLineFeed();
        auto jValue = rpos::system::serialization::json::Serializer<T>::serialize(value);

        auto str = writer.write(jValue);

        set(key, str);
    }

    template< class T >
    T Metadata::get(const std::string& key) const
    {
        Json::Reader reader;
        Json::Value value;

        auto str = get(key);

        if (!reader.parse(str, value))
        {
            return T();
        }

        return rpos::system::serialization::json::Serializer<T>::deserialize(value);
    }

    template< class T>
    bool Metadata::tryGet(const std::string& key, T& outValue) const
    {
        Json::Reader reader;
        Json::Value value;
        std::string str;

        if(!tryGet(key, str))
        {
            return false;
        }

        if (!reader.parse(str, value))
        {
            return false;
        }

        try
        {
            outValue = rpos::system::serialization::json::Serializer<T>::deserialize(value);
        }
        catch (...)
        {
            return false;
        }

        return true;
    }

}}
