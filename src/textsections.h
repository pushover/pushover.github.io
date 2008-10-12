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
