#pragma once

class Database
{
public:
	virtual void createObject(/* TODO: Args */);
	virtual void deleteObject(/* TODO: Args */);
	virtual void selectObject(/* TODO: Args */);
	virtual void updateObject(/* TODO: Args */);
	virtual void updateObjectIfEquals(/* TODO: Args */);

	virtual void selectQuery(/* TODO: Args */);
	virtual void updateQuery(/* TODO: Args */);
	virtual void deleteQuery(/* TODO: Args */);
};