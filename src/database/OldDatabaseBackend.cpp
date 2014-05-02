#include "OldDatabaseBackend.h"

void OldDatabaseBackend::submit(DBOperation *operation)
{
	if(operation->m_type == DBOperation::OperationType::CREATE_OBJECT)
	{
		ObjectData dbo(operation->m_dclass->get_id());
		dbo.fields = operation->m_set_fields;

		doid_t doid = create_object(dbo);
		if(doid == INVALID_DO_ID || doid < m_min_id || doid > m_max_id)
		{
			operation->on_failure();
		}
		else
		{
			operation->on_complete(doid);
		}
	}
	else if(operation->m_type == DBOperation::OperationType::DELETE_OBJECT)
	{
		delete_object(operation->m_doid);
		operation->on_complete();
	}
}
