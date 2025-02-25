//  Copyright Vladimir Prus 2004.
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt
//  or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/program_options/config.hpp>
#include <hpx/datastructures/any.hpp>
#include <hpx/program_options/detail/cmdline.hpp>
#include <hpx/program_options/detail/convert.hpp>
#include <hpx/program_options/value_semantic.hpp>

#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace hpx::program_options {

    inline std::string strip_prefixes(std::string const& text)
    {
        // "--foo-bar" -> "foo-bar"
        std::string::size_type i = text.find_first_not_of("-/");
        if (i == std::string::npos)
        {
            return text;
        }
        return text.substr(i);
    }

    namespace {

        std::string convert_value(std::wstring const& s)
        {
            try
            {
                return to_local_8_bit(s);
            }
            catch (std::exception const&)
            {
                return "<unrepresentable unicode string>";
            }
        }
    }    // namespace

    void value_semantic_codecvt_helper<char>::parse(
        hpx::any_nonser& value_store,
        std::vector<std::string> const& new_tokens, bool utf8) const
    {
        if (utf8)
        {
            // Need to convert to local encoding.
            std::vector<std::string> local_tokens;
            for (auto const& new_token : new_tokens)
            {
                std::wstring w = from_utf8(new_token);
                local_tokens.emplace_back(to_local_8_bit(w));
            }
            xparse(value_store, local_tokens);
        }
        else
        {
            // Already in local encoding, pass unmodified
            xparse(value_store, new_tokens);
        }
    }

    void value_semantic_codecvt_helper<wchar_t>::parse(
        hpx::any_nonser& value_store,
        std::vector<std::string> const& new_tokens, bool utf8) const
    {
        std::vector<std::wstring> tokens;
        if (utf8)
        {
            // Convert from utf8
            for (auto const& new_token : new_tokens)
            {
                tokens.emplace_back(from_utf8(new_token));
            }
        }
        else
        {
            // Convert from local encoding
            for (auto const& new_token : new_tokens)
            {
                tokens.emplace_back(from_local_8_bit(new_token));
            }
        }

        xparse(value_store, tokens);
    }

    std::string arg("arg");

    std::string untyped_value::name() const
    {
        return arg;
    }

    unsigned untyped_value::min_tokens() const noexcept
    {
        if (m_zero_tokens)
            return 0;
        else
            return 1;
    }

    unsigned untyped_value::max_tokens() const noexcept
    {
        return min_tokens();
    }

    void untyped_value::xparse(hpx::any_nonser& value_store,
        std::vector<std::string> const& new_tokens) const
    {
        if (value_store.has_value())
            throw multiple_occurrences();
        if (new_tokens.size() > 1)
            throw multiple_values();
        value_store = new_tokens.empty() ? std::string() : new_tokens.front();
    }

    typed_value<bool>* bool_switch()
    {
        return bool_switch(nullptr);
    }

    typed_value<bool>* bool_switch(bool* v)
    {
        typed_value<bool>* r = new typed_value<bool>(v);
        r->default_value(false);
        r->zero_tokens();

        return r;
    }

    /* Validates bool value.
        hpx::any_nonser of "1", "true", "yes", "on" will be converted to "1".
        hpx::any_nonser of "0", "false", "no", "off" will be converted to "0".
        Case is ignored. The 'xs' vector can either be empty, in which
        case the value is 'true', or can contain explicit value.
    */
    void validate(
        hpx::any_nonser& v, std::vector<std::string> const& xs, bool*, int)
    {
        check_first_occurrence(v);
        std::string s(get_single_string(xs, true));

        for (char& i : s)
            i = char(std::tolower(i));

        if (s.empty() || s == "on" || s == "yes" || s == "1" || s == "true")
            v = hpx::any_nonser(true);
        else if (s == "off" || s == "no" || s == "0" || s == "false")
            v = hpx::any_nonser(false);
        else
            throw invalid_bool_value(s);
    }

    // This is blatant copy-paste. However, templating this will cause a problem,
    // since wstring can't be constructed/compared with char*. We'd need to
    // create auxiliary 'widen' routine to convert from char* into
    // needed string type, and that's more work.
    HPX_CORE_EXPORT
    void validate(
        hpx::any_nonser& v, std::vector<std::wstring> const& xs, bool*, int)
    {
        check_first_occurrence(v);
        std::wstring s(get_single_string(xs, true));

        for (wchar_t& i : s)
            i = wchar_t(tolower(i));

        if (s.empty() || s == L"on" || s == L"yes" || s == L"1" || s == L"true")
            v = hpx::any_nonser(true);
        else if (s == L"off" || s == L"no" || s == L"0" || s == L"false")
            v = hpx::any_nonser(false);
        else
            throw invalid_bool_value(convert_value(s));
    }

    HPX_CORE_EXPORT
    void validate(hpx::any_nonser& v, std::vector<std::string> const& xs,
        std::string*, int)
    {
        check_first_occurrence(v);
        v = hpx::any_nonser(get_single_string(xs));
    }

    HPX_CORE_EXPORT
    void validate(hpx::any_nonser& v, std::vector<std::wstring> const& xs,
        std::string*, int)
    {
        check_first_occurrence(v);
        v = hpx::any_nonser(get_single_string(xs));
    }

    namespace validators {

        HPX_CORE_EXPORT
        void check_first_occurrence(hpx::any_nonser const& value)
        {
            if (value.has_value())
                throw multiple_occurrences();
        }
    }    // namespace validators

    invalid_option_value::invalid_option_value(std::string const& bad_value)
      : validation_error(validation_error::invalid_option_value)
    {
        set_substitute("value", bad_value);
    }

    invalid_option_value::invalid_option_value(std::wstring const& bad_value)
      : validation_error(validation_error::invalid_option_value)
    {
        set_substitute("value", convert_value(bad_value));
    }

    invalid_bool_value::invalid_bool_value(std::string const& bad_value)
      : validation_error(validation_error::invalid_bool_value)
    {
        set_substitute("value", bad_value);
    }

    error_with_option_name::error_with_option_name(std::string const& template_,
        std::string const& option_name, std::string const& original_token,
        int option_style)
      : error(template_)
      , m_option_style(option_style)
      , m_error_template(template_)
    {
        //     parameter            |     placeholder               |   value
        //     ---------            |     -----------               |   -----
        set_substitute_default(
            "canonical_option", "option '%canonical_option%'", "option");
        set_substitute_default("value", "argument ('%value%')", "argument");
        set_substitute_default("prefix", "%prefix%", "");
        m_substitutions["option"] = option_name;
        m_substitutions["original_token"] = original_token;
    }

    char const* error_with_option_name::what() const noexcept
    {
        // will substitute tokens each time what is run()
        substitute_placeholders(m_error_template);

        return m_message.c_str();
    }

    void error_with_option_name::replace_token(
        std::string const& from, std::string const& to) const
    {
        for (;;)
        {
            std::size_t pos = m_message.find(from);
            // not found: all replaced
            if (pos == std::string::npos)
                return;
            m_message.replace(pos, from.length(), to);
        }
    }

    std::string error_with_option_name::get_canonical_option_prefix() const
    {
        switch (m_option_style)
        {
        case command_line_style::allow_dash_for_short:
            [[fallthrough]];
        case command_line_style::allow_long_disguise:
            return "-";
        case command_line_style::allow_slash_for_short:
            return "/";
        case command_line_style::allow_long:
            return "--";
        case 0:
            return "";
        }
        throw std::logic_error(
            "error_with_option_name::m_option_style can only be "
            "one of [0, allow_dash_for_short, allow_slash_for_short, "
            "allow_long_disguise or allow_long]");
    }

    std::string error_with_option_name::get_canonical_option_name() const
    {
        auto option_it = m_substitutions.find("option");
        auto original_it = m_substitutions.find("original_token");
        if (option_it != m_substitutions.end() && !option_it->second.length())
        {
            return original_it != m_substitutions.end() ? original_it->second :
                                                          std::string();
        }

        std::string original_token;
        if (original_it != m_substitutions.end())
            original_token = strip_prefixes(original_it->second);

        std::string option_name;
        if (option_it != m_substitutions.end())
            option_name = strip_prefixes(option_it->second);

        //  For long options, use option name
        if (m_option_style == command_line_style::allow_long ||
            m_option_style == command_line_style::allow_long_disguise)
            return get_canonical_option_prefix() + option_name;

        //  For short options use first letter of original_token
        if (m_option_style && original_token.length())
            return get_canonical_option_prefix() + original_token[0];

        // no prefix
        return option_name;
    }

    void error_with_option_name::substitute_placeholders(
        std::string const& error_template) const
    {
        m_message = error_template;
        std::map<std::string, std::string> substitutions(m_substitutions);
        substitutions["canonical_option"] = get_canonical_option_name();
        substitutions["prefix"] = get_canonical_option_prefix();

        //
        //  replace placeholder with defaults if values are missing
        //
        for (auto const& substitution_default : m_substitution_defaults)
        {
            // missing parameter: use default
            if (substitutions.count(substitution_default.first) == 0 ||
                substitutions[substitution_default.first].length() == 0)
            {
                replace_token(substitution_default.second.first,
                    substitution_default.second.second);
            }
        }

        //
        //  replace placeholder with values
        //  placeholder are denoted by surrounding '%'
        //
        for (auto& substitution : substitutions)
            replace_token('%' + substitution.first + '%', substitution.second);
    }

    void ambiguous_option::substitute_placeholders(
        std::string const& original_error_template) const
    {
        // For short forms, all alternatives must be identical, by
        //      definition, to the specified option, so we don't need to
        //      display alternatives
        if (m_option_style == command_line_style::allow_dash_for_short ||
            m_option_style == command_line_style::allow_slash_for_short)
        {
            error_with_option_name::substitute_placeholders(
                original_error_template);
            return;
        }

        std::string error_template = original_error_template;
        // remove duplicates using std::set
        std::set<std::string> alternatives_set(
            m_alternatives.begin(), m_alternatives.end());
        std::vector<std::string> alternatives_vec(
            alternatives_set.begin(), alternatives_set.end());

        error_template += " and matches ";
        // Being very cautious: should be > 1 alternative!
        if (alternatives_vec.size() > 1)
        {
            for (std::size_t i = 0; i < alternatives_vec.size() - 1; ++i)
                error_template += "'%prefix%" + alternatives_vec[i] + "', ";
            error_template += "and ";
        }

        // there is a programming error if multiple options have the same name...
        if (m_alternatives.size() > 1 && alternatives_vec.size() == 1)
            error_template += "different versions of ";

        error_template += "'%prefix%" + alternatives_vec.back() + "'";

        // use inherited logic
        error_with_option_name::substitute_placeholders(error_template);
    }

    std::string validation_error::get_template(kind_t kind)
    {
        // Initially, store the message in 'const char*' variable,
        // to avoid conversion to std::string in all cases.
        char const* msg;
        switch (kind)
        {
        case invalid_bool_value:
            msg = "the argument ('%value%') for option '%canonical_option%' is "
                  "invalid. Valid choices are 'on|off', 'yes|no', '1|0' and "
                  "'true|false'";
            break;
        case invalid_option_value:
            msg = "the argument ('%value%') for option '%canonical_option%' is "
                  "invalid";
            break;
        case multiple_values_not_allowed:
            msg = "option '%canonical_option%' only takes a single argument";
            break;
        case at_least_one_value_required:
            msg = "option '%canonical_option%' requires at least one argument";
            break;
        // currently unused
        case invalid_option:
            msg = "option '%canonical_option%' is not valid";
            break;
        default:
            msg = "unknown error";
        }
        return msg;
    }
}    // namespace hpx::program_options
