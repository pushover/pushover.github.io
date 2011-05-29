/* Pushover
 *
 * Pushover is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335 USA
 */

#ifndef __TEXTSECTIONS_H__
#define __TEXTSECTIONS_H__

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <iostream>

class format_error : public std::exception {
  public:
    const std::string msg;
    format_error(const std::string & msg) throw(): msg(msg) {};
    virtual ~format_error() throw() {};
    virtual const char * what() const throw() { return msg.c_str(); }
};

class textsections_c {
  private:
    std::map<std::string, std::vector<std::vector<std::string> > > sections;
  public:
    static const std::string firstLine;
    textsections_c(std::istream & stream, bool singleFile);
    const std::vector<std::vector<std::string> > &
      getMultiSection(const std::string sectionName) const;
    const std::vector<std::string> &
      getSingleSection(const std::string sectionName) const;
    const std::string &
      getSingleLine(const std::string sectionName) const;
};

#endif
