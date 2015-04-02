// ============================================================================
/// @file
/// @brief This file contains the definition of the specialised polymorphic 
/// Visitor 
///
/// $Id$
/// $Copyright$
///
/// @copyright
/// @history
/// Reference   Who              When        What
///    632            Mike Manchip     07-June-2007 Original development
/// based on Jerry Lawsons original Visitor pattern
/// @endhistory
///
// ============================================================================

#if !defined (VisitorInterface_H_NG)
#define VisitorInterface_H_NG

#include <cal/defs.h>
#include <iostream>
#include <typeinfo>

namespace QAppNG
{

    class VisitorInterface
        {
    public:
        /// @brief  Default constructor

                        VisitorInterface ( void ) { }

        /// @brief  Virtual destructor

        virtual     ~   VisitorInterface ( ) { }

        /// @brief  Allow a visit from a Visitable object
        ///
        ///         It is meant that a VisitorInterface will define and implement its own
        ///         Visit() interface(s), depending upon what objects it is meant
        ///         to allow to be visited.
        ///
        /// @param[in] pVisitable   The object that is to be visited by this Visitor
        /// @return    boolean      The return value which is passed back through the
        ///                         Visitor's Accept(), and which is meaningful to the
        ///                         caller of Visitor::Accept ().

        virtual bool    Visit   ( class VisitableInterface * /* pVisitable */ ) { return true; };
        };

    /// @brief  Generic VisitableInterface
    ///
    /// The VisitableInterface is to be used as a base class to allow the subclass to "accept" visitors.
    /// The Visitor must support some form of Visit() that uses Visitable (or its subclass thereof)
    /// as an input argument.
    ///

    class VisitableInterface
        {
    public:

            VisitableInterface() {}

        virtual ~VisitableInterface() {}

        /// @brief  Accept a visitor.
        ///
        ///         This implements the double-dispatch to the pVisitor's
        ///         Visit() implementation. A Visitable subclass may override this function
        ///         and do other processing before, after, or instead of calling the Visitor's
        ///         Visit() function.
        ///

        template <typename VISITABLE, typename VISITOR>
        bool            Accept    ( VISITOR* pVisitor , VISITABLE *casttype)
                            {
                            CAL_UNUSED_ARG(casttype);
                            VISITABLE * pT = static_cast <VISITABLE *> (this);    // cast to the given type 
                            return pVisitor->Visit ( pT );
                            }

        };

    /// @brief  Generic VisitableRegistrar 
    ///
    /// The VisitableRegistrar is to be used as a base class to allow the registration of Visitables.
    ///

    class VisitableRegistrar
    {
      public:
        VisitableRegistrar() 
            {}

        virtual ~VisitableRegistrar() {}

            /// @brief  Register a Visitable  
            ///
            /// Provides an abstract interface for registering Visitables
        ///
            virtual void RegisterVisitable(VisitableInterface *visitable)=0;

    };



}
#endif // VisitorInterface_H_NG

// end of file.
