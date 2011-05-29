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

#include "textsections.h"

const std::string textsections_c::firstLine = "#!/usr/bin/env pushover";

textsections_c::textsections_c(std::istream & stream, bool singleFile) {

  std::vector<std::string> * currentSection = NULL;
  bool atFirstLine = true;
  for (;;) {

    std::streamsize pos = stream.tellg();

    std::string line;
    if (!std::getline(stream, line)) {
      /* end of stream */
      if (!stream.eof())
        throw format_error("unexpected stream error,"
                           " maybe the file does not exist");
      else
        break;
    }

    if (atFirstLine) {
      /* first line */
      if (line != firstLine)
        throw format_error("missing #!-line");
      atFirstLine = false;

    } else if (line == firstLine) {
      /* non-first #!-line */
      if (singleFile)
        throw format_error("unexpected #!-line");
      stream.clear();
      stream.seekg(pos);
      break;

    } else if (line.size() == 0 || line[0] == '+') {
      /* ignored line */

    } else if (line.size() > 0 && line[0] == '|') {
      /* section content */
      if (currentSection == NULL)
        throw format_error("section content before first section name");
      if (line.size() >= 2)
        currentSection->push_back(line.substr(2, std::string::npos));
      else
        currentSection->push_back("");

    } else {
      /* section name */
      sections[line].push_back(std::vector<std::string>());
      currentSection = &sections[line].back();
    }
  }
}

static const std::vector<std::vector<std::string> > empty;

const std::vector<std::vector<std::string> > &
  textsections_c::getMultiSection(const std::string sectionName) const {

  std::map<std::string, std::vector<std::vector<std::string> > >
    ::const_iterator i = sections.find(sectionName);
  if (i == sections.end())
    return empty;
  else
    return i->second;
}

const std::vector<std::string> &
  textsections_c::getSingleSection(const std::string sectionName) const {

  const std::vector<std::vector<std::string> > & section =
    getMultiSection(sectionName);

  if (section.size() == 1)
    return section.front();
  else if (section.empty())
    throw format_error("section \"" + sectionName + "\""
                       " is missing");
  else
    throw format_error("section \"" + sectionName + "\""
                       " occurs more than once");
}

const std::string &
  textsections_c::getSingleLine(const std::string sectionName) const {

  const std::vector<std::string> & singleSection =
    getSingleSection(sectionName);

  if (singleSection.size() == 1)
    return singleSection.front();
  else if (singleSection.empty())
    throw format_error("section \"" + sectionName + "\""
                       " is empty");
  else
    throw format_error("section \"" + sectionName + "\""
                       " has more than one line");
}
