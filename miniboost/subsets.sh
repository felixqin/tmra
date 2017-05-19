
THISDIR=`dirname $0`

BOOSTDIR=boost_1_61_0

cd ${THISDIR}
cd ${BOOSTDIR}

if [ ! -f dist/bin/bcp ]; then
	./bootstrap.sh
	./b2 tools/bcp
fi

rm out -fr
mkdir -p out
dist/bin/bcp	\
	make_shared.hpp	\
	boost/enable_shared_from_this.hpp	\
	boost/weak_ptr.hpp	\
	function.hpp	\
	bind.hpp	\
	unordered_map	\
	unordered_set	\
	boost/tuple/tuple.hpp \
	boost/scoped_ptr.hpp \
	boost/property_tree/ini_parser.hpp \
	boost/property_tree/json_parser.hpp \
	boost/algorithm/string.hpp \
	boost/date_time.hpp \
	boost/scoped_array.hpp \
	boost/shared_array.hpp \
	boost/thread.hpp	\
	boost/asio.hpp	\
	boost/uuid/uuid_io.hpp	\
	boost/uuid/uuid_generators.hpp	\
	boost/archive/binary_iarchive.hpp	\
	boost/archive/binary_oarchive.hpp	\
	out



