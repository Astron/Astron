// Filename: Struct.h
#pragma once
#include "Declaration.h"
#include "DistributedType.h"
#include "Field.h"
#include <string>        // std::string
#include <vector>        // std::vector
#include <unordered_map> // std::unordered_map
namespace dclass   // open namespace
{


// Foward declarations
class HashGenerator;
class File;

// A Struct provides DataType composition by combining multiple Fields.
//     Structs may have both anonymous and named Fields.
class Struct : public Declaration, public DistributedType
{
	public:
		Struct(File* file, const std::string &name);
		~Struct();

		// as_struct returns the same pointer converted to a struct pointer,
		//     if this is in fact a class; otherwise, returns NULL.
		virtual Struct* as_struct();
		virtual const Struct* as_struct() const;

		// get_file returns the File object that contains the struct.
		inline File* get_file() const;
		// get_name returns the name of this struct.
		inline const std::string& get_name() const;
		// get_id returns a unique index number associated with this struct.
		//     This is defined implicitly when a .dc file is read.
		inline unsigned int get_id() const;
		// get_num_fields returns the number of fields defined directly in the struct.
		inline size_t get_num_fields() const;
		// get_field returns the <n>th field defined directly in the struct or NULL if out-of-range.
		inline Field* get_field(unsigned int n) const;
		// get_field_by_name returns the field with <name>, or NULL if no such field exists.
		inline Field* get_field_by_name(const std::string& name) const;
		// get_field_by_id returns the field with the index <id>, or NULL if no such field exists.
		inline Field* get_field_by_id(unsigned int id) const;

		// set_id sets the index number associated with this struct.
		inline void set_id(unsigned int id);

		// add_field adds a new Field to the struct.
		virtual bool add_field(Field* field);

		// generate_hash accumulates the properties of this type into the hash.
		virtual void generate_hash(HashGenerator &hashgen) const;

		virtual void output(std::ostream &out) const;
		virtual void output(std::ostream &out, bool brief) const;
		virtual void write(std::ostream &out, bool brief, int indent_level) const;
		void output_instance(std::ostream &out, bool brief, const std::string &prename,
		                     const std::string &name, const std::string &postname) const;

	protected:
		Struct(); // Parameter constructor

		File *m_file;
		std::string m_name;
		unsigned int m_id;

		std::vector<Field*> m_fields;
		std::unordered_map<std::string, Field*> m_fields_by_name;
		std::unordered_map<unsigned int, Field*> m_fields_by_id;
};


} // close namespace dclass
#include "Struct.ipp"
