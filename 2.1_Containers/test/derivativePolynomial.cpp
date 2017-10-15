#include <string>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <algorithm>

int readNumber(const char **input) {
    /* Let stdio read it for us. */
    int number;
    int charsRead;
    int itemsRead;

    itemsRead = sscanf(*input, "%d%n", &number, &charsRead);

    if(itemsRead == 0) {
        return 1;
    }

    if(itemsRead < 0) {
        // Parse error.
        return -1;
    }

    *input += charsRead;

    return number;
}

int readExponent(const char **input) {
    if(strncmp("x^", *input, 2) != 0) {
        if(strncmp("x", *input, 1) == 0) {
            *input += 1;
            return 1;
	}
	exit(-1);
    }

    *input += 2;

    return readNumber(input);
}

/* aka skipWhitespaceAndPlus */
int readToNextToken(const char **input) {
    int ret = 1;
    while(**input && (isspace(**input) || **input == '-'  || **input == '+')) {
        if(!isspace(**input))
        {
            if( **input != '+')
            ret = -1;
            else
            ret = 1;
        }
        ++*input;
    }
    return ret;
}

void readTerm(const char **input, int &coefficient, int &exponent, bool &success) {
    success = false;

    int sign = readToNextToken(input);

    if(!**input) {
        return;
    }

    coefficient = sign * readNumber(input);

    sign = readToNextToken(input);

    if(!**input) {
        // Parse error.
        return;
    }

    exponent = sign * readExponent(input);

    success = true;
}

std::string derivative(std::string polynomial) {
    polynomial.erase(std::remove(polynomial.begin(), polynomial.end(), '*'),
              polynomial.end());
    std::string ret;
    std::map<int, int> polyMap;
    const char* input = polynomial.c_str();
    bool success = true;

    while(success) {
        int coefficient = 0, exponent = 0;

        readTerm(&input, coefficient, exponent, success);

        if(success) {
            if( polyMap.count(exponent) )
               polyMap[exponent] += coefficient;
            else
               polyMap[exponent] = coefficient;
        }
    }

    for ( auto it = polyMap.crbegin(); it != polyMap.crend(); ++it)
    {
        const auto& kvp = *it;
        int newCoef = kvp.first* kvp.second;
        if( newCoef < 0 && ret.size() ) ret.pop_back();
        ret += std::to_string(newCoef);
        if( 1 < kvp.first  && newCoef != 1) ret += "*"; 
	if(2 == kvp.first) ret += "x";
	else if(1 != kvp.first) ret += "x^" + std::to_string(-1 + kvp.first);
        ret += "+";
    }
    ret.pop_back();

    return ret;
}

int main(int argc, char** argv ) {
    for(int i = 0; i < argc; ++i) {
       if(i) std::cout <<  derivative(argv[i]) << std::endl;
    }
    return 0;
}
