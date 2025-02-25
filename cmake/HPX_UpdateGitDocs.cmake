# Copyright (c)      2020 Mikael Simberg
# Copyright (c) 2011-2013 Thomas Heller
#
# SPDX-License-Identifier: BSL-1.0
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

find_package(Git)

if(NOT GIT_FOUND)
  message(
    FATAL_ERROR
      "Could not find git. git is needed to download and update the GitHub pages."
  )
endif()

if(NOT GIT_REPOSITORY)
  set(GIT_REPOSITORY git@github.com:STEllAR-GROUP/hpx-docs.git --branch master)
endif()

if(EXISTS "${HPX_BINARY_DIR}/docs/gh-pages")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" pull --rebase
    WORKING_DIRECTORY "${HPX_BINARY_DIR}/docs/gh-pages"
    RESULT_VARIABLE git_pull_result
    ERROR_VARIABLE git_pull_result_message COMMAND_ECHO STDERR
  )
  if(NOT "${git_pull_result}" EQUAL "0")
    message(
      FATAL_ERROR
        "Updating the GitHub pages branch failed: ${git_pull_result_message}."
    )
  endif()
else()
  message("${GIT_EXECUTABLE}")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone ${GIT_REPOSITORY} gh-pages
    RESULT_VARIABLE git_clone_result
    ERROR_VARIABLE git_pull_result_message COMMAND_ECHO STDERR
  )
  if(NOT "${git_clone_result}" EQUAL "0")
    message(
      FATAL_ERROR
        "Cloning the GitHub pages branch failed: ${git_pull_result_message}. Trying to clone ${GIT_REPOSITORY}"
    )
  endif()
endif()

# We copy the documentation files from DOCS_SOURCE
set(DOCS_SOURCE "${HPX_BINARY_DIR}/share/hpx/docs")

# Turn the string of output formats back into a list
string(REGEX REPLACE " " ";" HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS
                     "${HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS}"
)

# If a branch name has been set, we copy files to a corresponding directory
message("HPX_WITH_GIT_BRANCH=\"${HPX_WITH_GIT_BRANCH}\"")
if(HPX_WITH_GIT_BRANCH)
  message("Updating branch directory")
  set(DOCS_BRANCH_DEST
      "${HPX_BINARY_DIR}/docs/gh-pages/branches/${HPX_WITH_GIT_BRANCH}"
  )
  file(REMOVE_RECURSE "${DOCS_BRANCH_DEST}")
  if("html" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
    file(
      COPY "${DOCS_SOURCE}/html"
      DESTINATION "${DOCS_BRANCH_DEST}"
      PATTERN "*.buildinfo" EXCLUDE
    )
  endif()
  if("singlehtml" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
    file(
      COPY "${DOCS_SOURCE}/singlehtml"
      DESTINATION "${DOCS_BRANCH_DEST}"
      PATTERN "*.buildinfo" EXCLUDE
    )
  endif()
  if("latexpdf" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
    if(EXISTS "${DOCS_SOURCE}/latexpdf/latex/HPX.pdf")
      file(COPY "${DOCS_SOURCE}/latexpdf/latex/HPX.pdf"
           DESTINATION "${DOCS_BRANCH_DEST}/pdf/"
      )
    endif()
  endif()
  # special handling of dependency report files
  if(EXISTS "${DOCS_SOURCE}/report")
    file(COPY "${DOCS_SOURCE}/report" DESTINATION "${DOCS_BRANCH_DEST}")
  endif()
endif()

# If a tag name has been set, we copy files to a corresponding directory
message("HPX_WITH_GIT_TAG=\"${HPX_WITH_GIT_TAG}\"")
if(HPX_WITH_GIT_TAG)
  message("Updating tag directory")
  set(DOCS_TAG_DEST "${HPX_BINARY_DIR}/docs/gh-pages/tags/${HPX_WITH_GIT_TAG}")
  file(REMOVE_RECURSE "${DOCS_TAG_DEST}")
  if("html" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
    file(
      COPY "${DOCS_SOURCE}/html"
      DESTINATION "${DOCS_TAG_DEST}"
      PATTERN "*.buildinfo" EXCLUDE
    )
  endif()
  if("singlehtml" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
    file(
      COPY "${DOCS_SOURCE}/singlehtml"
      DESTINATION "${DOCS_TAG_DEST}"
      PATTERN "*.buildinfo" EXCLUDE
    )
  endif()
  if("latexpdf" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
    file(
      COPY "${DOCS_SOURCE}/latexpdf/latex/HPX.pdf"
      DESTINATION "${DOCS_TAG_DEST}/pdf/"
      PATTERN "*.buildinfo" EXCLUDE
    )
  endif()

  # special handling of dependency report files
  if(EXISTS "${DOCS_SOURCE}/report")
    file(COPY "${DOCS_SOURCE}/report" DESTINATION "${DOCS_TAG_DEST}")
  endif()

  # If a tag name has been set and it is a suitable version number, we also copy
  # files to the "latest" directory. The regex only matches full version numbers
  # with three numerical components (X.Y.Z). It does not match release
  # candidates or other non-version tag names.
  if("${HPX_WITH_GIT_TAG}" MATCHES "^[0-9]+\\.[0-9]+\\.[0-9]+$")
    message("Updating latest directory")
    set(DOCS_LATEST_DEST "${HPX_BINARY_DIR}/docs/gh-pages/latest")
    file(REMOVE_RECURSE "${DOCS_LATEST_DEST}")
    if("html" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
      file(
        COPY "${DOCS_SOURCE}/html"
        DESTINATION "${DOCS_LATEST_DEST}"
        PATTERN "*.buildinfo" EXCLUDE
      )
    endif()
    if("singlehtml" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
      file(
        COPY "${DOCS_SOURCE}/singlehtml"
        DESTINATION "${DOCS_LATEST_DEST}"
        PATTERN "*.buildinfo" EXCLUDE
      )
    endif()
    if("latexpdf" IN_LIST HPX_WITH_DOCUMENTATION_OUTPUT_FORMATS)
      file(
        COPY "${DOCS_SOURCE}/latexpdf/latex/HPX.pdf"
        DESTINATION "${DOCS_LATEST_DEST}/pdf/"
        PATTERN "*.buildinfo" EXCLUDE
      )
    endif()

    # special handling of dependency report files
    if(EXISTS "${DOCS_SOURCE}/report")
      file(COPY "${DOCS_SOURCE}/report" DESTINATION "${DOCS_LATEST_DEST}")
    endif()
  endif()
endif()

# add all newly generated files
execute_process(
  COMMAND "${GIT_EXECUTABLE}" add *
  WORKING_DIRECTORY "${HPX_BINARY_DIR}/docs/gh-pages"
  RESULT_VARIABLE git_add_result
  ERROR_VARIABLE git_pull_result_message COMMAND_ECHO STDERR
)
if(NOT "${git_add_result}" EQUAL "0")
  message(
    FATAL_ERROR
      "Adding files to the GitHub pages branch failed: ${git_pull_result_message}."
  )
endif()

# check if there are changes to commit
execute_process(
  COMMAND "${GIT_EXECUTABLE}" diff-index --quiet HEAD
  WORKING_DIRECTORY "${HPX_BINARY_DIR}/docs/gh-pages"
  RESULT_VARIABLE git_diff_index_result COMMAND_ECHO STDERR
)
if(NOT "${git_diff_index_result}" EQUAL "0")
  # commit changes
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" commit -am "Updating Sphinx docs"
    WORKING_DIRECTORY "${HPX_BINARY_DIR}/docs/gh-pages"
    RESULT_VARIABLE git_commit_result
    ERROR_VARIABLE git_pull_result_message COMMAND_ECHO STDERR
  )
  if(NOT "${git_commit_result}" EQUAL "0")
    message(
      FATAL_ERROR
        "Committing to the GitHub pages branch failed: ${git_pull_result_message}."
    )
  endif()

  # push everything up to github
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" push
    WORKING_DIRECTORY "${HPX_BINARY_DIR}/docs/gh-pages"
    RESULT_VARIABLE git_push_result
    ERROR_VARIABLE git_pull_result_message COMMAND_ECHO STDERR
  )
  if(NOT "${git_push_result}" EQUAL "0")
    message(
      FATAL_ERROR
        "Pushing to the GitHub pages branch failed: ${git_pull_result_message}."
    )
  endif()
endif()
