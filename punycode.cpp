// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#include <future>
#include <sstream>

#include <daw/char_range/daw_char_range.h>

#include "punycode.h"

namespace daw {
	namespace {
		struct constants final {
			static size_t const BASE = 36;
			static size_t const TMIN = 1;
			static size_t const TMAX = 26;
			static size_t const SKEW = 38;
			static size_t const DAMP = 700;
			static size_t const INITIAL_BIAS = 72;
			static size_t const INITIAL_N = 128;
			constexpr static auto const PREFIX = "xn--";
			static char const DELIMITER = '-';
		};	// constants

		struct processed_cp_t {
			std::vector<uint32_t> all;
			std::vector<uint32_t> basic;
			std::vector<uint32_t> non_basic;
		};	// processed_cp_t

		processed_cp_t process_cp( daw::range::CharRange input ) {
			processed_cp_t result{ };
			for( auto const & cp : input ) {
				result.all.push_back( cp );
				if( cp < 128 ) {
					result.basic.push_back( cp );
				} else {
					result.non_basic.push_back( cp );
				}
			}
			return result;
		}

		template<typename T>
		auto adapt( T delta, size_t num_points, bool first_time ) {
			if( first_time ) {
				delta /= constants::DAMP;
			} else {
				delta /= 2;
			}
			delta += delta/num_points;

			T k = 0;
			while( delta > ((constants::BASE - constants::TMIN)* constants::TMAX) / 2 ) {
				delta /= constants::BASE - constants::TMIN;
				k += constants::BASE;
			}

			k += ((constants::BASE - constants::TMIN + 1	) * delta) / (delta + constants::SKEW);

			return k;
		}

		template<typename Iterator>
		auto sort_uniq( Iterator first, Iterator last ) {
			using value_type = std::decay_t<decltype(*first)>;
			std::vector<value_type> result;
			std::copy( first, last, std::back_inserter( result ) );
			std::sort( result.begin( ), result.end( ) );
			auto new_end = std::unique( result.begin( ), result.end( ) );
			result.erase( new_end, result.end( ) );
			return result;
		}


		template<typename T, typename U>
		auto calculate_threshold( T k, U bias ) {
			if( k <= bias + constants::TMIN ) {
				return constants::TMIN;
			} else if( k >= bias + constants::TMAX ) {
				return constants::TMAX;
			}
			return k - bias;
		}

		char encode_table( uint32_t pos ) {
			if( pos < 26 ) {
				return 'a' + static_cast<char>(pos);
			} else if ( pos < 36 ) {
				return '0' + static_cast<char>(pos);
			}
			throw std::runtime_error( "Invalid character to encode" );
		}

		std::string encode_part( daw::range::CharRange input ) {
			std::stringstream ss;

			auto n = constants::INITIAL_N;
			auto bias = constants::INITIAL_BIAS;
			size_t delta = 0;

			auto code_points = process_cp( input );
			auto h = code_points.basic.size( );
			auto b = code_points.basic.size( );
			for( auto const & cp: code_points.basic ) {
				ss << static_cast<char>( cp );
			}
			if( code_points.basic.size( ) == code_points.all.size( ) ) {
				return ss.str( );
			}
			if( b > 0 ) {
				ss << constants::DELIMITER;
			}
			code_points.non_basic = sort_uniq( code_points.non_basic.begin( ), code_points.non_basic.end( ) );

			auto non_basic_it = code_points.non_basic.begin( );

			auto length = code_points.all.size( );
			while( h < length ) {
				auto m = *(non_basic_it++);
				delta += (m-n)*(h + 1);
				n = m;

				for( auto const & c : code_points.all ) {
					if( c < n || c < constants::INITIAL_N ) {
						++delta;
					}
					if( c == n ) {
						auto q = delta;
						for( auto k = constants::BASE;; k += constants::BASE ) {
							auto const t = calculate_threshold( k, bias );
							if( q < t ) {
								break;
							}
							auto const code = t + ((q - t) % (constants::BASE -t));
							ss << encode_table( code );

							q = (q - t)/(constants::BASE -t);
						}

						ss << encode_table( q );
						bias = adapt( delta, h + 1, (h == b) );
						delta = 0;
						++h;
					}
				}
				++delta;
				++n;
			}
			std::string result= constants::PREFIX + ss.str( );
			if( result.empty( ) || result.size( ) > 63 ) {
				throw std::runtime_error( "The length of any one label is limited to between 1 and 63 octets" );
			}
			return result;
		}
	}    // namespace anonymous

	template<typename Delemiter>
	std::vector<boost::string_ref> split( boost::string_ref input, Delemiter delemiter ) {
		std::vector<boost::string_ref> result;
		auto pos = input.find_first_of( delemiter );
		while( !input.empty( ) && boost::string_ref::npos != pos ) {
			result.emplace_back( input.data( ), pos );
			input = input.substr( pos + 1, boost::string_ref::npos );
			pos = input.find_first_of( delemiter );
		}
		result.push_back( input );
		return result;
	}

	std::string to_puny_code( boost::string_ref input ) {
		std::vector<std::future<std::string>> workers;
		auto parts = split( input, '.' );
		for( auto const & part : parts ) {
			workers.emplace_back( std::async( std::launch::async, [part]( ) {
				if( part.empty( )) {
					return std::string{ };
				}
				return encode_part( daw::range::create_char_range( part.begin( ), part.end( )));
			} ));
		}
		std::stringstream ss;
		// join the strings
		bool is_first = true;
		for( auto & wrk : workers ) {
			if( !is_first ) {
				ss << ".";
			} else {
				is_first = false;
			}
			ss << wrk.get( );
		}
		return ss.str( );
	}

}    // namespace daw
