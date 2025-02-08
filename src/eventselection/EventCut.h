/*!
 * \file EventCut.h
 * \brief Base class for event cuts
 * \author Tanner Mengel <tmengel@bnl.gov>
 * \version $Verison: 1.0.1 $
 * \date $Date: 09/10/2024 $
 */

#ifndef EVENTSELECTION_EVENTCUT_H
#define EVENTSELECTION_EVENTCUT_H

#include <iostream>
#include <string>
#include <vector>
#include <utility>

class PHCompositeNode;

class EventCut
{
  public:

    virtual ~EventCut() {}

    virtual void identify(std::ostream &os = std::cout) const { os << "EventCut::" << Name() << std::endl; }
    
    void clear(){ Passed(false); }
  
    // setters
    void Passed(bool result) { m_result = result; }
    void Name(const std::string &name) { m_name = name; }

    // getters
    bool Passed() const { return m_result; }
    const std::string& Name() const { return m_name; }

    int Verbosity() const { return m_verbosity; }
    void Verbosity(int verbosity) { m_verbosity = verbosity; }

    virtual bool operator()(PHCompositeNode* topNode) = 0;

    void SetNodeNames(const std::vector<std::string> & names) { m_node_names = names; }
    const std::vector<std::string>& GetNodeNames() const { return m_node_names; }
    void SetNodeName(const std::string &name) { SetNodeNames({ name }); }
    const std::string& GetNodeName() const { return GetNodeNames().front(); }

    void SetRange(float high, float low) { m_var_range = std::make_pair(high, low); }
    std::pair<float, float> GetRange() const { return m_var_range; }



  protected:
    
    EventCut( const std::string &name = "EventCut" ) :  m_name(name) {}

  private:

    int m_verbosity{0};

    std::string m_name {""};
    bool m_result{false};

    std::vector<std::string> m_node_names {};

    // only used for cuts that need a range
    std::pair<float, float> m_var_range{0, 0};
};

#endif // EVENTSELECTION_EVENTCUT_H