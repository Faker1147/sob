#ifndef __SOB_KEY_VAL__
#define __SOB_KEY_VAL__

#include <vector>
#include <fstream>
#include <memory>

constexpr int SOB_MAX_KV_LEN = 128; //Was 256 [APG]RoboCop[CL]

class CSOBKeyValue
{
public:
	CSOBKeyValue(const char* szKey, const char* szValue);

	const char* getKey() const { return m_szKey; }
	const char* getValue() const { return m_szValue; }
private:
	char m_szKey[SOB_MAX_KV_LEN];
	char m_szValue[SOB_MAX_KV_LEN];
};

class CSOBKeyValueList
{
public:
	~CSOBKeyValueList() = default;

	void parseFile(std::fstream& fp);

	CSOBKeyValue* getKV(const char* key) const;

	bool getFloat(const char* key, float* val) const;
	bool getInt(const char* key, int* val) const;
	bool getString(const char* key, const char** val) const;
private:
	std::vector<std::unique_ptr<CSOBKeyValue>> m_KVs;
};

#endif