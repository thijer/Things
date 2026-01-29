#ifndef UTILITY_H
#define UTILITY_H

// Compile debug printing functionality if THING_DEBUG is set to the desired output `Print` instance
#ifdef THING_DEBUG
void PRINT();

void PRINT(uint8_t* str, unsigned int length)
{
    THING_DEBUG.write(str, length);
    THING_DEBUG.print("\n");
}

template<typename T, typename... Args>
void PRINT(T t, Args... args)
{
    THING_DEBUG.print(t);
    PRINT(args...);
}

void PRINT()
{
    THING_DEBUG.print('\n');
}
#else

#define PRINT(...) ;

#endif


#endif