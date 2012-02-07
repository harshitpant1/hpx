//  Copyright (c) 2011 Bryce Lelbach
//  Copyright (c) 2007-2012 Hartmut Kaiser
//  Copyright (c) 2007 Richard D. Guidry Jr.
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_NAMING_NAME_MAR_24_2008_0942AM)
#define HPX_NAMING_NAME_MAR_24_2008_0942AM

#include <ios>
#include <iomanip>
#include <iostream>

#include <boost/io/ios_state.hpp>
#include <boost/cstdint.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/detail/atomic_count.hpp>

#include <hpx/config.hpp>
#include <hpx/exception.hpp>
#include <hpx/util/safe_bool.hpp>
#include <hpx/util/spinlock_pool.hpp>
#include <hpx/runtime/naming/address.hpp>

#include <hpx/config/warnings_prefix.hpp>

///////////////////////////////////////////////////////////////////////////////
// Version of id_type
#define HPX_IDTYPE_VERSION  0x20
#define HPX_GIDTYPE_VERSION 0x10

///////////////////////////////////////////////////////////////////////////////
namespace hpx { namespace naming
{
    ///////////////////////////////////////////////////////////////////////////
    /// Global identifier for components across the HPX system.
    struct HPX_EXPORT gid_type
    {
        // These typedefs are for Boost.ICL.
        typedef gid_type size_type;
        typedef gid_type difference_type;

        static boost::uint64_t const credit_base_mask = 0x7ffful;
        static boost::uint64_t const credit_mask = credit_base_mask << 16;
        static boost::uint64_t const was_split_mask = 0x80000000ul;

        explicit gid_type (boost::uint64_t lsb_id = 0)
          : id_msb_(0), id_lsb_(lsb_id)
        {}

        explicit gid_type (boost::uint64_t msb_id, boost::uint64_t lsb_id)
          : id_msb_(msb_id), id_lsb_(lsb_id)
        {}

        gid_type& operator=(boost::uint64_t lsb_id)
        {
            id_msb_ = 0;
            id_lsb_ = lsb_id;
            return *this;
        }

        operator util::safe_bool<gid_type>::result_type() const
        {
            return util::safe_bool<gid_type>()(0 != id_lsb_ || 0 != id_msb_);
        }

        // We support increment, decrement, addition and subtraction
        gid_type& operator++()       // pre-increment
        {
            *this += 1;
            return *this;
        }
        gid_type operator++(int)     // post-increment
        {
            gid_type t(*this);
            ++(*this);
            return t;
        }

        gid_type& operator--()       // pre-decrement
        {
            *this -= 1;
            return *this;
        }
        gid_type operator--(int)     // post-decrement
        {
            gid_type t(*this);
            ++(*this);
            return t;
        }

        // GID + GID
        friend gid_type operator+ (gid_type const& lhs, gid_type const& rhs)
        {
            boost::uint64_t lsb = lhs.id_lsb_ + rhs.id_lsb_;
            boost::uint64_t msb = lhs.id_msb_ + rhs.id_msb_;
            if (lsb < lhs.id_lsb_ || lsb < rhs.id_lsb_)
                ++msb;
            return gid_type(msb, lsb);
        }
        gid_type operator+= (gid_type const& rhs)
        { return (*this = *this + rhs); }

        // GID + boost::uint64_t
        friend gid_type operator+ (gid_type const& lhs, boost::uint64_t rhs)
        { return lhs + gid_type(0, rhs); }
        gid_type operator+= (boost::uint64_t rhs)
        { return (*this = *this + rhs); }

        // GID - GID
        friend gid_type operator- (gid_type const& lhs, gid_type const& rhs)
        {
            boost::uint64_t lsb = lhs.id_lsb_ - rhs.id_lsb_;
            boost::uint64_t msb = lhs.id_msb_ - rhs.id_msb_;
            if (lsb > lhs.id_lsb_)
                --msb;
            return gid_type(msb, lsb);
        }
        gid_type operator-= (gid_type const& rhs)
        { return (*this = *this - rhs); }

        // GID - boost::uint64_t
        friend gid_type operator- (gid_type const& lhs, boost::uint64_t rhs)
        { return lhs - gid_type(0, rhs); }
        gid_type operator-= (boost::uint64_t rhs)
        { return (*this = *this - rhs); }

        friend gid_type operator& (gid_type const& lhs, boost::uint64_t rhs)
        {
            return gid_type(lhs.id_msb_, lhs.id_lsb_ & rhs);
        }

        // comparison is required as well
        friend bool operator== (gid_type const& lhs, gid_type const& rhs)
        {
            return (lhs.id_msb_ == rhs.id_msb_) && (lhs.id_lsb_ == rhs.id_lsb_);
        }
        friend bool operator!= (gid_type const& lhs, gid_type const& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator< (gid_type const& lhs, gid_type const& rhs)
        {
            if (lhs.id_msb_ < rhs.id_msb_)
                return true;
            if (lhs.id_msb_ > rhs.id_msb_)
                return false;
            return lhs.id_lsb_ < rhs.id_lsb_;
        }

        friend bool operator<= (gid_type const& lhs, gid_type const& rhs)
        {
            if (lhs.id_msb_ < rhs.id_msb_)
                return true;
            if (lhs.id_msb_ > rhs.id_msb_)
                return false;
            return lhs.id_lsb_ <= rhs.id_lsb_;
        }

        friend bool operator> (gid_type const& lhs, gid_type const& rhs)
        {
            if (lhs.id_msb_ > rhs.id_msb_)
                return true;
            if (lhs.id_msb_ < rhs.id_msb_)
                return false;
            return lhs.id_lsb_ > rhs.id_lsb_;
        }

        friend bool operator>= (gid_type const& lhs, gid_type const& rhs)
        {
            if (lhs.id_msb_ > rhs.id_msb_)
                return true;
            if (lhs.id_msb_ < rhs.id_msb_)
                return false;
            return lhs.id_lsb_ >= rhs.id_lsb_;
        }

        boost::uint64_t get_msb() const
        {
            return id_msb_;
        }
        void set_msb(boost::uint64_t msb)
        {
            id_msb_ = msb;
        }
        boost::uint64_t get_lsb() const
        {
            return id_lsb_;
        }
        void set_lsb(boost::uint64_t lsb)
        {
            id_lsb_ = lsb;
        }
        void set_lsb(void* lsb)
        {
            id_lsb_ = reinterpret_cast<boost::uint64_t>(lsb);
        }

        struct tag {};
        typedef hpx::util::spinlock_pool<tag> mutex_type;

    private:
        friend std::ostream& operator<< (std::ostream& os, gid_type const& id);

        friend class boost::serialization::access;

        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & id_msb_;
            ar & id_lsb_;
        }

        // actual gid
        boost::uint64_t id_msb_;
        boost::uint64_t id_lsb_;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline std::ostream& operator<< (std::ostream& os, gid_type const& id)
    {
        boost::io::ios_flags_saver ifs(os);
        os << std::hex
           << "{" << std::right << std::setfill('0') << std::setw(16)
                  << id.id_msb_ << ", "
                  << std::right << std::setfill('0') << std::setw(16)
                  << id.id_lsb_ << "}";
        return os;
    }

    ///////////////////////////////////////////////////////////////////////////
    //  Handle conversion to/from prefix
    inline gid_type get_gid_from_prefix(boost::uint32_t prefix) HPX_SUPER_PURE;

    inline gid_type get_gid_from_prefix(boost::uint32_t prefix)
    {
        return gid_type(boost::uint64_t(prefix+1) << 32, 0);
    }

    inline boost::uint32_t get_prefix_from_gid(gid_type const& id) HPX_PURE;

    inline boost::uint32_t get_prefix_from_gid(gid_type const& id)
    {
        return boost::uint32_t(id.get_msb() >> 32)-1;
    }

    inline gid_type get_locality_from_gid(gid_type const& id)
    {
        return get_gid_from_prefix(get_prefix_from_gid(id));
    }

    ///////////////////////////////////////////////////////////////////////////
    inline boost::uint16_t get_credit_from_gid(gid_type const& id) HPX_PURE;

    inline boost::uint16_t get_credit_from_gid(gid_type const& id)
    {
        return boost::uint16_t((id.get_msb() & gid_type::credit_mask) >> 16);
    }

    // has side effects, can't be pure
    inline boost::uint16_t add_credit_to_gid(gid_type& id, boost::uint16_t credit)
    {
        boost::uint64_t msb = id.get_msb();
        boost::uint32_t c =
            boost::uint16_t((msb & gid_type::credit_mask) >> 16) + credit;

        BOOST_ASSERT(0 == (c & ~gid_type::credit_base_mask));
        id.set_msb((msb & ~gid_type::credit_mask) |
            ((c & gid_type::credit_base_mask) << 16));
        return c;
    }

    inline boost::uint64_t strip_credit_from_gid(boost::uint64_t msb) HPX_SUPER_PURE;

    inline boost::uint64_t strip_credit_from_gid(boost::uint64_t msb)
    {
        return msb & ~(gid_type::credit_mask | gid_type::was_split_mask);
    }

    inline void strip_credit_from_gid(gid_type& id)
    {
        id.set_msb(strip_credit_from_gid(id.get_msb()));
    }

    inline gid_type strip_credit_from_gid(gid_type const& id) HPX_PURE;

    inline gid_type strip_credit_from_gid(gid_type const& id)
    {
        boost::uint64_t const msb = strip_credit_from_gid(id.get_msb());
        boost::uint64_t const lsb = id.get_lsb();
        return gid_type(msb, lsb);
    }

    inline gid_type strip_credit_from_cgid(gid_type const& id) HPX_PURE;

    inline gid_type strip_credit_from_cgid(gid_type const& id)
    {
        boost::uint64_t const msb = strip_credit_from_gid(id.get_msb());
        boost::uint64_t const lsb = id.get_lsb();
        return gid_type(msb, lsb);
    }

    inline void set_credit_for_gid(gid_type& id, boost::uint16_t credit)
    {
        BOOST_ASSERT(0 == (credit & ~gid_type::credit_base_mask));
        id.set_msb((id.get_msb() & ~gid_type::credit_mask) |
            ((credit & gid_type::credit_base_mask) << 16));
    }

    inline gid_type split_credits_for_gid(gid_type& id, int fraction = 2)
    {
        boost::uint64_t msb = id.get_msb();
        boost::uint16_t credits = boost::uint16_t((msb & gid_type::credit_mask) >> 16);
        boost::uint32_t newcredits = credits / fraction;

        msb &= ~gid_type::credit_mask;
        id.set_msb(msb | (((credits - newcredits) << 16) & gid_type::credit_mask) | gid_type::was_split_mask);

        return gid_type(msb | ((newcredits << 16) & gid_type::credit_mask) | gid_type::was_split_mask,
            id.get_lsb());
    }

    inline bool gid_was_split(gid_type const& id)
    {
        return (id.get_msb() & gid_type::was_split_mask) ? true : false;
    }

    ///////////////////////////////////////////////////////////////////////
    inline bool is_local_address(gid_type const& gid, gid_type const& prefix) HPX_PURE;

    inline bool is_local_address(gid_type const& gid, gid_type const& prefix)
    {
        return strip_credit_from_gid(gid.get_msb()) == prefix.get_msb();
    }

    ///////////////////////////////////////////////////////////////////////////
    gid_type const invalid_gid = gid_type();

    namespace detail
    {
        ///////////////////////////////////////////////////////////////////////
        enum id_type_management
        {
            unknown_deleter = -1,
            unmanaged = 0,          // unmanaged GID
            managed = 1,            // managed GID
            transmission = 2        // special deleter for temporaries created
                                    // inside the parcelhandler
        };

        // forward declaration
        struct HPX_EXPORT id_type_impl;

        // custom deleter for id_type_impl above
        void HPX_EXPORT gid_managed_deleter (id_type_impl* p);
        void HPX_EXPORT gid_unmanaged_deleter (id_type_impl* p);
        void HPX_EXPORT gid_transmission_deleter (id_type_impl* p);

        ///////////////////////////////////////////////////////////////////////
        struct HPX_EXPORT id_type_impl : gid_type
        {
        private:
            typedef void (*deleter_type)(detail::id_type_impl*);

            static deleter_type get_deleter(id_type_management t)
            {
                switch (t) {
                case unmanaged:
                    return &detail::gid_unmanaged_deleter;
                case managed:
                    return &detail::gid_managed_deleter;
                case transmission:
                    return &detail::gid_transmission_deleter;
                default:
                    BOOST_ASSERT(false);          // invalid management type
                    return &detail::gid_unmanaged_deleter;
                };
                return 0;
            }

        public:
            explicit id_type_impl (boost::uint64_t lsb_id, id_type_management t)
              : gid_type(0, lsb_id), count_(0), type_(t)
            {}

            explicit id_type_impl (boost::uint64_t msb_id, boost::uint64_t lsb_id,
                    id_type_management t)
              : gid_type(msb_id, lsb_id), count_(0), type_(t)
            {}

            explicit id_type_impl (gid_type const& gid, id_type_management t)
              : gid_type(gid), count_(0), type_(t)
            {}

            id_type_management get_management_type() const
            {
                return type_;
            }

//             explicit id_type_impl (boost::uint64_t msb_id, boost::uint64_t lsb_id,
//                 locality const& l, naming::address::component_type type,
//                 naming::address::address_type a)
//               : gid_type(msb_id, lsb_id), address_(l, type, a)
//             {}
//
//             bool resolve(naming::address& addr);
//             bool is_resolved() const { return address_; }
//
//             bool resolve_c();
//             bool resolve_c(naming::address& addr);
//
//             bool get_local_address(naming::address& addr)
//             {
//                 if (!is_local_cached() && !resolve())
//                     return false;
//                 gid_type::mutex_type::scoped_lock l(this);
//                 addr = address_;
//                 return true;
//             }
//
//             bool get_local_address_c_cache(naming::address& addr)
//             {
//                 if (!is_local_cached() && !resolve_c())
//                     return false;
//                 gid_type::mutex_type::scoped_lock l(this);
//                 addr = address_;
//                 return true;
//             }
//
//             bool get_address_cached(naming::address& addr) const
//             {
//                 if (!is_cached())
//                     return false;
//                 gid_type::mutex_type::scoped_lock l(this);
//                 addr = address_;
//                 return true;
//             }
//
//             // cached resolved address
//             naming::address address_;
//
//         protected:
//             bool is_local_cached();
//             bool is_cached() const;
//
//             bool is_local();
//             bool is_local_c_cache();
//
//             bool resolve();

        private:
            friend void intrusive_ptr_add_ref(id_type_impl* p);
            friend void intrusive_ptr_release(id_type_impl* p);

            boost::detail::atomic_count count_;
            id_type_management type_;
        };

        /// support functions for boost::intrusive_ptr
        inline void intrusive_ptr_add_ref(id_type_impl* p)
        {
            ++p->count_;
        }

        inline void intrusive_ptr_release(id_type_impl* p)
        {
            if (0 == --p->count_)
                id_type_impl::get_deleter(p->get_management_type())(p);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // the local gid is actually just a wrapper around the real thing
    struct HPX_EXPORT id_type
    {
    public:
        enum management_type
        {
            unknown_deleter = detail::unknown_deleter,
            unmanaged = detail::unmanaged,          ///< unmanaged GID
            managed = detail::managed,              ///< managed GID
            transmission = detail::transmission     ///< special deleter for temporaries created
                                                    ///< inside the parcelhandler
        };

        id_type() {}

        explicit id_type(boost::uint64_t lsb_id, management_type t)
          : gid_(new detail::id_type_impl(0, lsb_id,
                static_cast<detail::id_type_management>(t)))
        {}

        explicit id_type(gid_type const& gid, management_type t)
          : gid_(new detail::id_type_impl(gid,
                static_cast<detail::id_type_management>(t)))
        {
            BOOST_ASSERT(get_credit_from_gid(*gid_) || t == unmanaged ||
                t == transmission);
        }

        explicit id_type(boost::uint64_t msb_id, boost::uint64_t lsb_id,
                management_type t)
          : gid_(new detail::id_type_impl(msb_id, lsb_id,
                static_cast<detail::id_type_management>(t)))
        {
            BOOST_ASSERT(get_credit_from_gid(*gid_) || t == unmanaged ||
                t == transmission);
        }

//         explicit id_type(boost::uint64_t msb_id, boost::uint64_t lsb_id,
//               locality const& l, naming::address::component_type type_,
//               naming::address::address_type a, id_type_management t/* = unmanaged*/)
//           : gid_(new detail::id_type_impl(msb_id, lsb_id, l, type_, a),
//                          get_deleter(t))
//         {
//             BOOST_ASSERT(get_credit_from_gid(*gid_) || t == unmanaged ||
//                          t == transmission);
//         }

        gid_type& get_gid() { return *gid_; }
        gid_type const& get_gid() const { return *gid_; }

//         id_type_management get_management_type() const;

        id_type& operator++()       // pre-increment
        {
            ++(*gid_);
            return *this;
        }
        id_type operator++(int)     // post-increment
        {
            (*gid_)++;
            return *this;
        }

        operator util::safe_bool<id_type>::result_type() const
        {
            return util::safe_bool<id_type>()(gid_ && *gid_);
        }

        // comparison is required as well
        friend bool operator== (id_type const& lhs, id_type const& rhs)
        {
            if (!lhs)
                return !rhs;
            if (!rhs)
                return !lhs;

            return *lhs.gid_ == *rhs.gid_;
        }
        friend bool operator!= (id_type const& lhs, id_type const& rhs)
        {
            return !(lhs == rhs);
        }

        friend bool operator< (id_type const& lhs, id_type const& rhs)
        {
            return lhs.gid_.get() < rhs.gid_.get();
        }

        friend bool operator<= (id_type const& lhs, id_type const& rhs)
        {
            return lhs.gid_.get() <= rhs.gid_.get();
        }

        friend bool operator> (id_type const& lhs, id_type const& rhs)
        {
            return lhs.gid_.get() > rhs.gid_.get();
        }

        friend bool operator>= (id_type const& lhs, id_type const& rhs)
        {
            return lhs.gid_.get() >= rhs.gid_.get();
        }

        boost::uint64_t get_msb() const
        {
            return gid_->get_msb();
        }
        void set_msb(boost::uint64_t msb)
        {
            gid_->set_msb(msb);
        }
        boost::uint64_t get_lsb() const
        {
            return gid_->get_lsb();
        }
        void set_lsb(boost::uint64_t lsb)
        {
            gid_->set_lsb(lsb);
        }
        void set_lsb(void* lsb)
        {
            gid_->set_lsb(lsb);
        }

        // functions for credit management
        boost::uint16_t get_credit() const
        {
            gid_type::mutex_type::scoped_lock l(gid_.get());
            return get_credit_from_gid(*gid_);
        }
        void strip_credit()
        {
            gid_type::mutex_type::scoped_lock l(gid_.get());
            strip_credit_from_gid(*gid_);
        }
        boost::uint16_t add_credit(boost::uint16_t credit)
        {
            gid_type::mutex_type::scoped_lock l(gid_.get());
            return add_credit_to_gid(*gid_, credit);
        }
        void set_credit(boost::uint16_t credit) const
        {
            gid_type::mutex_type::scoped_lock l(gid_.get());
            set_credit_for_gid(*gid_, credit);
        }
        id_type split_credits(int fraction = 2) const
        {
            gid_type::mutex_type::scoped_lock l(gid_.get());
            return id_type(split_credits_for_gid(*gid_, fraction), transmission);
        }
        bool was_split() const
        {
            gid_type::mutex_type::scoped_lock l(gid_.get());
            return gid_was_split(*gid_);
        }

        ///////////////////////////////////////////////////////////////////////
//         bool is_local_cached() const
//         {
//             return gid_->is_local_cached();
//         }
//         bool is_local() const
//         {
//             return gid_->is_local();
//         }
//         bool is_local_c_cache() const
//         {
//             return gid_->is_local_c_cache();
//         }
//         bool get_local_address(naming::address& addr) const
//         {
//             return gid_->get_local_address(addr);
//         }
//         bool get_local_address_c_cache(naming::address& addr) const
//         {
//             return gid_->get_local_address_c_cache(addr);
//         }
//         bool get_address_cached(naming::address& addr) const
//         {
//             return gid_->get_address_cached(addr);
//         }
//
//         bool resolve(address& addr)
//         {
//             return gid_->resolve(addr);
//         }
//         bool is_resolved() const
//         {
//             return gid_->is_resolved();
//         }

    private:
        friend std::ostream& operator<< (std::ostream& os, id_type const& id);

        friend class boost::serialization::access;

        template <class Archive>
        void save(Archive & ar, const unsigned int version) const;

        template <class Archive>
        void load(Archive & ar, const unsigned int version);

        BOOST_SERIALIZATION_SPLIT_MEMBER()

        boost::intrusive_ptr<detail::id_type_impl> gid_;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline std::ostream& operator<< (std::ostream& os, id_type const& id)
    {
        os << id.get_gid();
        return os;
    }

    ///////////////////////////////////////////////////////////////////////
    //  Handle conversion to/from prefix
    inline id_type get_id_from_prefix(boost::uint32_t prefix) HPX_SUPER_PURE;

    inline id_type get_id_from_prefix(boost::uint32_t prefix)
    {
        return id_type(boost::uint64_t(prefix+1) << 32, 0, id_type::unmanaged);
    }

    inline boost::uint32_t get_prefix_from_id(id_type const& id) HPX_PURE;

    inline boost::uint32_t get_prefix_from_id(id_type const& id)
    {
        return boost::uint32_t(id.get_msb() >> 32)-1;
    }

    inline id_type get_locality_from_id(id_type const& id)
    {
        return get_id_from_prefix(get_prefix_from_id(id));
    }

    ///////////////////////////////////////////////////////////////////////
    inline bool is_local_address(id_type const& gid, id_type const& prefix) HPX_PURE;

    inline bool is_local_address(id_type const& gid, id_type const& prefix)
    {
        return is_local_address(gid.get_gid(), prefix.get_gid());
    }

    ///////////////////////////////////////////////////////////////////////
    id_type const invalid_id = id_type();

    ///////////////////////////////////////////////////////////////////////
    HPX_EXPORT char const* get_management_type_name(id_type::management_type m);
}}

///////////////////////////////////////////////////////////////////////////////
// this is the current version of the id_type serialization format
BOOST_CLASS_VERSION(hpx::naming::gid_type, HPX_GIDTYPE_VERSION)
BOOST_CLASS_TRACKING(hpx::naming::gid_type, boost::serialization::track_never)
BOOST_CLASS_VERSION(hpx::naming::id_type, HPX_IDTYPE_VERSION)
BOOST_CLASS_TRACKING(hpx::naming::id_type, boost::serialization::track_never)

#include <hpx/config/warnings_suffix.hpp>

#endif

