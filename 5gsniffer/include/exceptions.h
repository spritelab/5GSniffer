/*
 * Copyright (c) 2021.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */

/**
 * @file exceptions.h
 *
 * @brief Defines custom exceptions that should be thrown for errors specific to 
 * this project.
 */

#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <stdexcept>
#include <sstream>
#include <memory>
#include <iostream>

/**
 * Generic exception for all errors related to this project. New exceptions
 * should all inherit from this class.
 */
struct sniffer_exception : public std::runtime_error {
  sniffer_exception(const std::string what_arg) : 
    std::runtime_error(what_arg) {
  }
};

/**
 * Exception used for all configuration-related errors.
 */
struct config_exception : public sniffer_exception {
  template <class T>
  config_exception(const T what_arg) : 
    sniffer_exception(std::string("Configuration exception: ") + std::string(what_arg)) {
  }
};

/**
 * Exception used for all SDR-related errors.
 */
struct sdr_exception : public sniffer_exception {
  template <class T>
  sdr_exception(const T what_arg) : 
    sniffer_exception(std::string("SDR exception: ") + std::string(what_arg)) {
  }
};

#endif
