/*ckwg +5
 * Copyright 2014 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

/**
 * \file
 * \brief Implementation of \link maptk::algo::close_loops_multi_method
 *        close_loops_multi_method \endlink
 */

#include <maptk/core/algo/close_loops_multi_method.h>
#include <maptk/core/algo/algorithm.txx>
#include <maptk/core/exceptions/algorithm.h>

#include <algorithm>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


namespace maptk
{

namespace algo
{


// Return IDs of all methods labels.
std::vector< std::string >
method_names( unsigned count )
{
  std::vector< std::string > output;

  for( unsigned i = 0; i < count; i++ )
  {
    output.push_back( "method" + boost::lexical_cast<std::string>( i+1 ) );
  }

  return output;
}


close_loops_multi_method
::close_loops_multi_method()
: count_( 1 ),
  methods_( 1 )
{
}


close_loops_multi_method
::close_loops_multi_method(const close_loops_multi_method& other)
: count_( other.count_ ),
  methods_( other.methods_.size() )
{
}


config_block_sptr
close_loops_multi_method
::get_configuration() const
{
  // Get base config from base class
  config_block_sptr config = algorithm::get_configuration();

  // Internal parameters
  config->set_value( "count", count_, "Number of close loops methods we want to use." );

  // Sub-algorithm implementation name + sub_config block
  std::vector< std::string > method_ids = method_names( count_ );

  for( unsigned i = 0; i < method_ids.size(); i++ )
  {
    close_loops::get_nested_algo_configuration( method_ids[i], config, methods_[i] );
  }

  return config;
}


void
close_loops_multi_method
::set_configuration( config_block_sptr in_config )
{
  // Starting with our generated config_block to ensure that assumed values are present
  // An alternative is to check for key presence before performing a get_value() call.
  config_block_sptr config = this->get_configuration();
  config->merge_config( in_config );

  // Parse count parameter
  count_ = config->get_value<unsigned>( "count" );
  methods_.resize( count_ );

  // Parse methods
  std::vector<std::string> method_ids = method_names( count_ );

  for( unsigned i = 0; i < method_ids.size(); i++ )
  {
    close_loops::set_nested_algo_configuration( method_ids[i], config, methods_[i] );
  }
}


bool
close_loops_multi_method
::check_configuration( config_block_sptr config ) const
{
  std::vector<std::string> method_ids = method_names( config->get_value<unsigned>( "count" ) );

  for( unsigned i = 0; i < method_ids.size(); i++ )
  {
    if( !close_loops::check_nested_algo_configuration( method_ids[i], config ) )
    {
      return false;
    }
  }

  return true;
}


track_set_sptr
close_loops_multi_method
::stitch( frame_id_t frame_number, image_container_sptr image, track_set_sptr input ) const
{
  track_set_sptr updated_set = input;

  for( unsigned i = 0; i < methods_.size(); i++ )
  {
    updated_set = methods_[i]->stitch( frame_number, image, updated_set );
  }

  return updated_set;
}


} // end namespace algo

} // end namespace maptk