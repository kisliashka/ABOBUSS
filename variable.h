#ifndef VERIABLE_H
#define VERIABLE_H
#include <iostream>
#include <string>

class Variable 
{
public:
	explicit Variable(std::string t_type, std::string t_value, int t_array_l) : type(t_type),
		value(t_value), array_l(t_array_l) {};
	std::string type;
	std::string value = "";
	int			array_l;
	int iterator = 0;


	bool operator==(const Variable& other) const {
		return type == other.type && value == other.value && array_l == other.array_l;
	}
};

#endif VERIABLE_H
