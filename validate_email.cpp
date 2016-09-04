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
#include <cstdint>
#include <future>
#include <thread>
#include <boost/asio.hpp>

#include <daw/daw_algorithm.h>
#include <daw/char_range/daw_char_range.h>

#include "punycode.h"
#include "validate_email.h"

namespace daw {
	namespace {

		auto is_local( daw::range::CharRange rng ) {
			// These are the only invalid characters.  Beyond this
			// the MTA has authority over whether it is valid or not
			// TL;DR no control characters

			// Copy to a u32 string so that element access is O(1) not O(n)
			std::u32string local_str;
			std::copy( rng.begin( ), rng.end( ), std::back_inserter( local_str ) );
			
			if( local_str.size( ) > 64 || local_str.empty( ) ) {
				return false;
			}
			using namespace daw::algorithm;
			if( satisfies_one( local_str.begin( ), local_str.end( ), in_range( 0, 31 ), equal_to( 127 ) ) ) {
				return false;
			}
			if( local_str.front( ) == U'.' ) {
				return false;
			}
			auto prev = U'\0';
			auto quote_count = std::accumulate( local_str.begin( ), local_str.end( ), static_cast<size_t>(0), [&prev]( auto & init, auto c ) {
				if( prev != U'\\' && c == U'"' ) {
					prev = c;
					return init + 1;
				}
				prev = c;
				return init;
			} );
			
			if( quote_count != 0 ) {
				// Should only have 0 or 2 quotes(escaped quotes aren't counted)
				if( quote_count != 2 ) {
					return false;
				}
				// Surrounded with quotes \"....\" 
				if( local_str.size( ) >= 3 && local_str[0] == U'\\' && local_str[1] == '"' && local_str.back( ) == U'"' ) {
					return false;
				}

			} else {
				// Should not beging or end in a .
				if( local_str[0] == U'.' || local_str.back( ) == U'.' ) {
					return false;
				}
				// Cannot have @ if not within quotes on whole local
				for( auto c: local_str ) {
					if( c == U'@' ) {
						return false;
					}
				}
			}
			return true;
		}

		auto can_resolve( daw::range::CharRange rng ) {
			using namespace boost::asio;
			io_service io_service;
			ip::tcp::resolver resolver{ io_service };
			auto str_puny = daw::to_puny_code( std::string( rng.raw_begin( ), rng.raw_end( ) ) );
			ip::tcp::resolver::query query{ str_puny, "" };
			boost::system::error_code ec;
			auto result = resolver.resolve( query, ec );
			return result != ip::tcp::resolver::iterator{ };
		}

		template<typename ForwardIterator, typename Value>
		auto find_last( ForwardIterator first, ForwardIterator last, Value val ) {
			auto result = last; 
			for( auto pos = first; pos != last; ++pos ) {
				if( *pos == val ) {
					result = pos;
				}
			}
			return result;
		}
	}	// namespace anonymous

	boost::string_ref get_local_part( boost::string_ref email_address ) noexcept {
		auto amp_pos = email_address.find_last_of( '@' );
		if( boost::string_ref::npos == amp_pos ) {
			return "";
		}
		return email_address.substr( 0, amp_pos );
	}

	boost::string_ref get_domain_part( boost::string_ref email_address ) noexcept {
		auto amp_pos = email_address.find_last_of( '@' );
		if( boost::string_ref::npos == amp_pos ) {
			return "";
		}
		auto result = email_address.substr( amp_pos + 1 );
		if( result.front( ) == '[' && result.back( ) == ']' ) {
			result = result.substr( 1, result.size( ) - 2 );
			// [127.0.0.1] is a valid domain, bracketed ip. 
			using namespace daw::algorithm;
			for( auto const & c: result ) {
				if( !satisfies_one( c, in_range( '0', '9' ), equal_to( '.' ) ) ) {
					return "";
				}
			}
		}
		return result;
	}

	bool is_email_address( boost::string_ref email_address ) {
		auto local_str = get_local_part( email_address );
		if( local_str.empty( ) ) {
			return false;
		}
		auto domain_str = get_domain_part( email_address );
		if( domain_str.empty( ) ) {
			return false;
		}
		if( daw::range::create_char_range( email_address.begin( ), email_address.end( ) ).size( ) > 255 ) {
			return false;
		}
		auto u_local_str = daw::range::create_char_range( local_str.begin( ), local_str.end( ) );
		auto u_domain_str = daw::range::create_char_range( domain_str.begin( ), domain_str.end( ) );

		auto domain_good = std::async( std::launch::async, [u_domain_str]( ) {
			return can_resolve( u_domain_str );
		} );

		return is_local( u_local_str ) && domain_good.get( );
	}
}	// namespace daw

