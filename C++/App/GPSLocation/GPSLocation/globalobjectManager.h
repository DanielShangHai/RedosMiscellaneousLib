#pragma once

class globalobjectManager
{
private:
	// Singleton implementation, constructors are private
	globalobjectManager(void);
	virtual ~globalobjectManager(void);
	globalobjectManager(globalobjectManager const&);
	globalobjectManager const& operator=(globalobjectManager const&);

public:
	static globalobjectManager* instance(); // return pointer to singleton object

public:


private:
	bool m_startDraw;
public:
	void enableStartDraw(bool enabled);
	bool enabledStartDraw(void);
};
