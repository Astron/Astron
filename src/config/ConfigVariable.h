#pragma once
#include "ConfigGroup.h"
#include <istream>    // std::istream
#include <list>       // std::list

// TODO: Doc
class ConfigFile
{
    template<typename T>
    friend class ConfigVariable;

  public:
    bool load(std::istream &is);

    ConfigNode copy_node();

  private:
    ConfigNode m_node;
};


// Foward declarations
extern std::unique_ptr<ConfigFile> g_config;
template<typename T>
class ConfigConstraint;
template<typename T>
class RawConfigConstraint;


// TODO: Doc
template<typename T>
class ConfigVariable
{
  private:
    std::string m_name;
    T m_def_val;
    ConfigGroup* m_group;

    std::list<ConfigConstraint<T>*> m_constraints;
    std::list<RawConfigConstraint<T>*> m_raw_constraints;

    // get_val_by_path gets the value at the given config path
    T get_val_by_path(ConfigNode node, std::string path)
    {
        size_t offset = 0;
        ConfigNode cnode = YAML::Clone(node);
        bool going = true;
        while(going) {
            size_t noffset = path.find("/", offset);
            if(noffset == std::string::npos) {
                going = false;
                break;
            } else {
                cnode = cnode[path.substr(offset, noffset - offset)];
                if(!cnode.IsDefined()) {
                    return m_def_val;
                }
            }
            offset = noffset + 1;
        }
        cnode = cnode[path.substr(offset, path.length() - offset)];
        if(!cnode.IsDefined()) {
            return m_def_val;
        }
        return cnode.as<T>();
    }

    // get_raw_by_path gets the raw text at the given config path
    std::string get_raw_by_path(ConfigNode node, std::string path)
    {
        size_t offset = 0;
        ConfigNode cnode = YAML::Clone(node);

        bool going = true;
        while(going) {
            size_t noffset = path.find("/", offset);
            if(noffset == std::string::npos) {
                going = false;
                break;
            } else {
                cnode = cnode[path.substr(offset, noffset - offset)];
                if(!cnode.IsDefined()) {
                    return "";
                }
            }
            offset = noffset + 1;
        }
        cnode = cnode[path.substr(offset, path.length() - offset)];

        if(!cnode.IsDefined() || !cnode.IsScalar()) {
            return "";
        }

        return cnode.Scalar();
    }

    bool rtest_constraints(ConfigNode node)
    {
        std::string raw = get_rraw(node);
        for(auto it = m_raw_constraints.begin(); it != m_raw_constraints.end(); ++it) {
            if(!(*it)->test(raw)) {
                config_error((*it)->error());
                return false;
            }
        }

        T val = get_rval(node);
        for(auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if(!(*it)->test(val)) {
                config_error((*it)->error());
                return false;
            }
        }
        return true;
    }

    bool test_constraints()
    {
        std::string raw = get_raw();
        for(auto it = m_raw_constraints.begin(); it != m_raw_constraints.end(); ++it) {
            if(!(*it)->test(raw)) {
                config_error((*it)->error());
                return false;
            }
        }

        T val = get_val();
        for(auto it = m_constraints.begin(); it != m_constraints.end(); ++it) {
            if(!(*it)->test(val)) {
                config_error((*it)->error());
                return false;
            }
        }

        return true;
    }

    rtest get_rtest()
    {
        return std::bind(&ConfigVariable::rtest_constraints, this, std::placeholders::_1);
    }

    test get_test()
    {
        return std::bind(&ConfigVariable::test_constraints, this);
    }

  public:
    ConfigVariable(const std::string& name, const T& def_val,
                   ConfigGroup* grp = &ConfigGroup::root()) :
        m_name(name), m_def_val(def_val), m_group(grp)
    {
        grp->add_variable(name, get_rtest());
    }

    ConfigVariable(const std::string& name, const T& def_val, ConfigGroup& grp) :
        m_name(name), m_def_val(def_val), m_group(&grp)
    {
        grp.add_variable(name, get_rtest());
    }

    // Get rval gets the value of the config variable relative to the variable's ConfigGroup.
    T get_rval(ConfigNode node)
    {
        return get_val_by_path(node, m_name);
    }

    // Get val gets the value of the config variable from the global config scope.
    T get_val()
    {
        return get_val_by_path(g_config->m_node, m_group->get_path() + m_name);
    }

    // Get rraw gets the raw text of the config variable relative to the variable's ConfigGroup.
    std::string get_rraw(ConfigNode node)
    {
        return get_raw_by_path(node, m_name);
    }

    // Get raw gets the raw text of the config variable from the global config scope.
    std::string get_raw()
    {
        return get_raw_by_path(g_config->m_node, m_group->get_path() + m_name);
    }

    void add_constraint(ConfigConstraint<T>* constraint)
    {
        m_constraints.push_back(constraint);
    }

    void add_raw_constraint(RawConfigConstraint<T>* constraint)
    {
        m_raw_constraints.push_back(constraint);
    }
};

// TODO: Doc
template<typename T>
class ConfigConstraint
{
  public:
    typedef bool (*function)(const T&);

  private:
    function m_test;
    std::string m_error;

  public:
    ConfigConstraint(function test, ConfigVariable<T>& var, const std::string& error) :
        m_test(test), m_error(error)
    {
        var.add_constraint(this);
    }

    bool test(const T& v)
    {
        return m_test(v);
    }

    std::string error()
    {
        return m_error;
    }
};

// TODO: Doc
template<typename T>
class RawConfigConstraint
{
  public:
    typedef bool (*function)(const std::string&);

  private:
    function m_test;
    std::string m_error;

  public:
    RawConfigConstraint(function test, ConfigVariable<T>& var, const std::string& error) :
        m_test(test), m_error(error)
    {
        var.add_raw_constraint(this);
    }

    bool test(const std::string& str)
    {
        return m_test(str);
    }

    std::string error()
    {
        return m_error;
    }
};
