/**
 * @file acta.h
 * @brief Account Services (act:a)
 */
#pragma once
#include <3ds/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /// Initializes ACTA service.
    Result actaInit();

    /// Exits ACTA.
    void actaExit();

    /// Get ACTA handle.
    Handle *actaGetSessionHandle();

#ifdef __cplusplus
}
#endif