#ifndef LIBKLIO_STORE_FACTORY_HPP
#define LIBKLIO_STORE_FACTORY_HPP 1

#include <boost/filesystem.hpp>
#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/store.hpp>


namespace bfs = boost::filesystem;

namespace klio {

    class StoreFactory {
    public:
        typedef std::tr1::shared_ptr<StoreFactory> Ptr;

        StoreFactory() {
        };

        virtual ~StoreFactory() {
        };

        /**
         * @deprecated
         */
        Store::Ptr create_store(const STORETYPE& type, const bfs::path& path);
        /**
         * @deprecated
         */
        Store::Ptr open_store(const STORETYPE& type, const bfs::path& path);

        Store::Ptr open_sqlite3_store(const bfs::path& path);
        Store::Ptr open_msg_store(const std::string& url);

    private:
        StoreFactory(const StoreFactory& original);
        StoreFactory& operator =(const StoreFactory& rhs);
    };
};

#endif /* LIBKLIO_STORE-FACTORY_HPP */
