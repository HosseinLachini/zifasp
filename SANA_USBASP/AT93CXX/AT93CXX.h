#ifndef AT93CXX_H_INCLUDED
#define AT93CXX_H_INCLUDED

#define		AT93C46			7			//	7 address
#define		AT93C56			8			//	8 address
#define		AT93C66			9			//	9 address
#define		AT93C76			10			// 10 address
#define		AT93C86			11			// 11 address
#define		NO_CHIP			0

extern unsigned char at93cxxEnterProgrammingMode();
extern void at93cxxConnect(void);
extern void at93cxxErasesAll();

#endif // 93CXX_H_INCLUDED
