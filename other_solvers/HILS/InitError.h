/*
 *
 * InitError: initialization exception class
 *
 *  Created on: 15/09/2015
 *      Author: bruno
 */


namespace opt
{

class InitError : public std::exception
{

public:

	InitError(const std::string & err) : what_(ArgPack::ap().program_name + ": " + err) {}

	virtual ~InitError() throw () {}

	virtual const char * what() const throw () { return what_.c_str(); }

private:

	std::string what_;

};

} // namespace opt
