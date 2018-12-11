#pragma once
/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/** @file
 *  @brief find bin/lib/data/conf in a 'qi' standard sdk layout
 */

#ifndef _SRC_SDKLAYOUT_HPP_
#define _SRC_SDKLAYOUT_HPP_

# include <vector>
# include <string>
# include <boost/noncopyable.hpp>
# include <qi/api.hpp>

/**
 * \namespace qi
 * \brief SDKLayout implementation.
 *
 * \warning Every string MUST be encoded in UTF8 and return UTF-8.
 *
 */
namespace qi
{
  namespace path
  {
    namespace detail
    {
      /// The suffix for executable files for your platform.
      std::string binSuffix();
    }
  }

  class PrivateSDKLayout;

  /** \class SDKLayout sdklayout.hpp "qi/path/sdklayout.hpp"
   *  \brief A class to handle SDK Layouts.
   *
   *  This class allow to get various path information:
   *   - user writable data and configuration path
   *   - library and binary path
   *   - static readonly configuration and data files
   */
  class QI_API SDKLayout : boost::noncopyable
  {
  public:

    /** \brief This constructor use argc/argv stored by qi::Application(argc, argv)
     *  to determine the sdk layout.
     *
     *  If qi::Application has not been called before creating this class,
     *  call to methods of this class will throw an exception.
     */
    SDKLayout();

    /** \brief Construct a SDKLayout using prefix as the sdk prefix.
     *  \param prefix a valid sdk prefix.
     *  \param mode "" by default, use to check sdk initialization.
     */
    explicit SDKLayout(const std::string &prefix, const std::string &mode = "");

    virtual ~SDKLayout();



    /** @copydoc qi::path::getSdkPrefix */
    std::string sdkPrefix() const;

    /** @copydoc qi::path::getSdkPrefixes */
    std::vector<std::string> getSdkPrefixes() const;

    /** @copydoc qi::path::addOptionalSdkPrefix */
    void addOptionalSdkPrefix(const char *prefix);

    /** @copydoc qi::path::clearOptionalSdkPrefix */
    void clearOptionalSdkPrefix();



    /** @copydoc qi::path::findBinary */
    std::string findBin(const std::string &name, bool searchInPath = false) const;

    /** @copydoc qi::path::findLibrary */
    std::string findLib(const std::string &name) const;

    /** @copydoc qi::path::findConfiguration */
    std::string findConf(const std::string &applicationName,
                         const std::string &filename,
                         bool excludeUserWritablePath = false) const;

    /** @copydoc qi::path::findData */
    std::string findData(const std::string &applicationName,
                         const std::string &filename,
                         bool excludeUserWritablePath = false) const;

    /** @copydoc qi::path::listData */
    std::vector<std::string> listData(const std::string &applicationName,
                                      const std::string &pattern="*",
                                      bool excludeUserWritablePath = false) const;

    /** @copydoc qi::path::listLib */
    std::vector<std::string> listLib(const std::string &applicationName,
                                     const std::string &pattern) const;

    /** @copydoc qi::path::getConfigurationPaths */
    std::vector<std::string> confPaths(const std::string &applicationName="",
                                       bool excludeUserWritablePath = false) const;

    /** @copydoc qi::path::getDataPaths */
    std::vector<std::string> dataPaths(const std::string &applicationName="",
                                       bool excludeUserWritablePath = false) const;

    /** @copydoc qi::path::getBinaryPaths */
    std::vector<std::string> binPaths(const std::string &subfolder="") const;

    /** @copydoc qi::path::getLibraryPaths */
    std::vector<std::string> libPaths(const std::string &subfolder="") const;


    /** @copydoc qi::path::detail::setWritablePath */
    void setWritablePath(const std::string &path);

    /** @copydoc qi::path::getUserWritableDataPath */
    std::string userWritableDataPath(const std::string &applicationName,
                                        const std::string &filename) const;

    /** @copydoc qi::path::getUserWritableConfigurationPath */
    std::string userWritableConfPath(const std::string &applicationName,
                                                 const std::string &filename="") const;
  private:
    // Pimpl
    PrivateSDKLayout* _p;
  };

}

#endif  // _SRC_SDKLAYOUT_HPP_
