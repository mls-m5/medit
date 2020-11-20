#pragma once

#include "syntax/ipalette.h"
#include "clang-c/Index.h"

namespace {

FormatType getFormat(CXCursorKind kind) {
    switch (kind) {
    case CXCursor_UnexposedDecl:
    case CXCursor_StructDecl:
    case CXCursor_UnionDecl:
    case CXCursor_ClassDecl:
    case CXCursor_EnumDecl:
    case CXCursor_FieldDecl:
    case CXCursor_EnumConstantDecl:
    case CXCursor_FunctionDecl:
    case CXCursor_VarDecl:
    case CXCursor_ParmDecl:
    case CXCursor_TypedefDecl:
    case CXCursor_CXXMethod:
        return IPalette::type;
    case CXCursor_Namespace:
    case CXCursor_LinkageSpec:
    case CXCursor_Constructor:
    case CXCursor_Destructor:
    case CXCursor_ConversionFunction:
    case CXCursor_TemplateTypeParameter:
    case CXCursor_NonTypeTemplateParameter:
    case CXCursor_TemplateTemplateParameter:
    case CXCursor_FunctionTemplate:
    case CXCursor_ClassTemplate:
    case CXCursor_ClassTemplatePartialSpecialization:
    case CXCursor_NamespaceAlias:
    case CXCursor_UsingDirective:
    case CXCursor_UsingDeclaration:
    case CXCursor_TypeAliasDecl:
    case CXCursor_ObjCSynthesizeDecl:
    case CXCursor_ObjCDynamicDecl:
    case CXCursor_CXXAccessSpecifier:
    case CXCursor_FirstRef:
    case CXCursor_ObjCProtocolRef:
    case CXCursor_ObjCClassRef:
    case CXCursor_TypeRef:
    case CXCursor_CXXBaseSpecifier:
    case CXCursor_TemplateRef:
    case CXCursor_NamespaceRef:
    case CXCursor_MemberRef:
    case CXCursor_LabelRef:
    case CXCursor_OverloadedDeclRef:
    case CXCursor_VariableRef:
    case CXCursor_FirstInvalid:
    case CXCursor_NoDeclFound:
    case CXCursor_NotImplemented:
    case CXCursor_InvalidCode:
    case CXCursor_FirstExpr:
    case CXCursor_DeclRefExpr:
    case CXCursor_MemberRefExpr:
    case CXCursor_CallExpr:
    case CXCursor_ObjCMessageExpr:
    case CXCursor_BlockExpr:
        return IPalette::identifier;
    case CXCursor_IntegerLiteral:
    case CXCursor_FloatingLiteral:
    case CXCursor_ImaginaryLiteral:
    case CXCursor_StringLiteral:
        return IPalette::string;
    case CXCursor_CharacterLiteral:
    case CXCursor_ParenExpr:
    case CXCursor_UnaryOperator:
    case CXCursor_ArraySubscriptExpr:
    case CXCursor_BinaryOperator:
    case CXCursor_CompoundAssignOperator:
    case CXCursor_ConditionalOperator:
    case CXCursor_CStyleCastExpr:
    case CXCursor_CompoundLiteralExpr:
    case CXCursor_InitListExpr:
    case CXCursor_AddrLabelExpr:
    case CXCursor_StmtExpr:
    case CXCursor_GenericSelectionExpr:
    case CXCursor_GNUNullExpr:
    case CXCursor_CXXStaticCastExpr:
    case CXCursor_CXXDynamicCastExpr:
    case CXCursor_CXXReinterpretCastExpr:
    case CXCursor_CXXConstCastExpr:
    case CXCursor_CXXFunctionalCastExpr:
    case CXCursor_CXXTypeidExpr:
    case CXCursor_CXXBoolLiteralExpr:
    case CXCursor_CXXNullPtrLiteralExpr:
    case CXCursor_CXXThisExpr:
    case CXCursor_CXXThrowExpr:
    case CXCursor_CXXNewExpr:
    case CXCursor_CXXDeleteExpr:
    case CXCursor_UnaryExpr:
    case CXCursor_ObjCStringLiteral:
    case CXCursor_ObjCEncodeExpr:
    case CXCursor_ObjCSelectorExpr:
    case CXCursor_ObjCProtocolExpr:
    case CXCursor_ObjCBridgedCastExpr:
    case CXCursor_PackExpansionExpr:
    case CXCursor_SizeOfPackExpr:
    case CXCursor_LambdaExpr:
    case CXCursor_ObjCBoolLiteralExpr:
    case CXCursor_ObjCSelfExpr:
    case CXCursor_OMPArraySectionExpr:
    case CXCursor_ObjCAvailabilityCheckExpr:
    case CXCursor_FixedPointLiteral:
    case CXCursor_FirstStmt:
    case CXCursor_LabelStmt:
    case CXCursor_CompoundStmt:
    case CXCursor_CaseStmt:
    case CXCursor_DefaultStmt:
    case CXCursor_IfStmt:
    case CXCursor_SwitchStmt:
    case CXCursor_WhileStmt:
    case CXCursor_DoStmt:
    case CXCursor_ForStmt:
    case CXCursor_GotoStmt:
    case CXCursor_IndirectGotoStmt:
    case CXCursor_ContinueStmt:
    case CXCursor_BreakStmt:
    case CXCursor_ReturnStmt:
    case CXCursor_GCCAsmStmt:
    case CXCursor_ObjCAtTryStmt:
    case CXCursor_ObjCAtCatchStmt:
    case CXCursor_ObjCAtFinallyStmt:
    case CXCursor_ObjCAtThrowStmt:
    case CXCursor_ObjCAtSynchronizedStmt:
    case CXCursor_ObjCAutoreleasePoolStmt:
    case CXCursor_ObjCForCollectionStmt:
    case CXCursor_CXXCatchStmt:
    case CXCursor_CXXTryStmt:
    case CXCursor_CXXForRangeStmt:
    case CXCursor_SEHTryStmt:
    case CXCursor_SEHExceptStmt:
    case CXCursor_SEHFinallyStmt:
    case CXCursor_MSAsmStmt:
    case CXCursor_NullStmt:
    case CXCursor_DeclStmt:
    case CXCursor_OMPParallelDirective:
    case CXCursor_OMPSimdDirective:
    case CXCursor_OMPForDirective:
    case CXCursor_OMPSectionsDirective:
    case CXCursor_OMPSectionDirective:
    case CXCursor_OMPSingleDirective:
    case CXCursor_OMPParallelForDirective:
    case CXCursor_OMPParallelSectionsDirective:
    case CXCursor_OMPTaskDirective:
    case CXCursor_OMPMasterDirective:
    case CXCursor_OMPCriticalDirective:
    case CXCursor_OMPTaskyieldDirective:
    case CXCursor_OMPBarrierDirective:
    case CXCursor_OMPTaskwaitDirective:
    case CXCursor_OMPFlushDirective:
    case CXCursor_SEHLeaveStmt:
    case CXCursor_OMPOrderedDirective:
    case CXCursor_OMPAtomicDirective:
    case CXCursor_OMPForSimdDirective:
    case CXCursor_OMPParallelForSimdDirective:
    case CXCursor_OMPTargetDirective:
    case CXCursor_OMPTeamsDirective:
    case CXCursor_OMPTaskgroupDirective:
    case CXCursor_OMPCancellationPointDirective:
    case CXCursor_OMPCancelDirective:
    case CXCursor_OMPTargetDataDirective:
    case CXCursor_OMPTaskLoopDirective:
    case CXCursor_OMPTaskLoopSimdDirective:
    case CXCursor_OMPDistributeDirective:
    case CXCursor_OMPTargetEnterDataDirective:
    case CXCursor_OMPTargetExitDataDirective:
    case CXCursor_OMPTargetParallelDirective:
    case CXCursor_OMPTargetParallelForDirective:
    case CXCursor_OMPTargetUpdateDirective:
    case CXCursor_OMPDistributeParallelForDirective:
    case CXCursor_OMPDistributeParallelForSimdDirective:
    case CXCursor_OMPDistributeSimdDirective:
    case CXCursor_OMPTargetParallelForSimdDirective:
    case CXCursor_OMPTargetSimdDirective:
    case CXCursor_OMPTeamsDistributeDirective:
    case CXCursor_OMPTeamsDistributeSimdDirective:
    case CXCursor_OMPTeamsDistributeParallelForSimdDirective:
    case CXCursor_OMPTeamsDistributeParallelForDirective:
    case CXCursor_OMPTargetTeamsDirective:
    case CXCursor_OMPTargetTeamsDistributeDirective:
    case CXCursor_OMPTargetTeamsDistributeParallelForDirective:
    case CXCursor_OMPTargetTeamsDistributeParallelForSimdDirective:
    case CXCursor_OMPTargetTeamsDistributeSimdDirective:
    case CXCursor_BuiltinBitCastExpr:
    case CXCursor_TranslationUnit:
    case CXCursor_FirstAttr:
    case CXCursor_IBActionAttr:
    case CXCursor_IBOutletAttr:
    case CXCursor_IBOutletCollectionAttr:
    case CXCursor_CXXFinalAttr:
    case CXCursor_CXXOverrideAttr:
    case CXCursor_AnnotateAttr:
    case CXCursor_AsmLabelAttr:
    case CXCursor_PackedAttr:
    case CXCursor_PureAttr:
    case CXCursor_ConstAttr:
    case CXCursor_NoDuplicateAttr:
    case CXCursor_CUDAConstantAttr:
    case CXCursor_CUDADeviceAttr:
    case CXCursor_CUDAGlobalAttr:
    case CXCursor_CUDAHostAttr:
    case CXCursor_CUDASharedAttr:
    case CXCursor_VisibilityAttr:
    case CXCursor_DLLExport:
    case CXCursor_DLLImport:
    case CXCursor_NSReturnsRetained:
    case CXCursor_NSReturnsNotRetained:
    case CXCursor_NSReturnsAutoreleased:
    case CXCursor_NSConsumesSelf:
    case CXCursor_NSConsumed:
    case CXCursor_FlagEnum:
    case CXCursor_ConvergentAttr:
    case CXCursor_WarnUnusedAttr:
    case CXCursor_WarnUnusedResultAttr:
    case CXCursor_AlignedAttr:
    case CXCursor_PreprocessingDirective:
    case CXCursor_MacroDefinition:
    case CXCursor_MacroExpansion:
    case CXCursor_InclusionDirective:
    case CXCursor_ModuleImportDecl:
    case CXCursor_TypeAliasTemplateDecl:
    case CXCursor_StaticAssert:
    case CXCursor_FriendDecl:
    case CXCursor_OverloadCandidate:

    default:
        break;
    }

    return 1;
}

} // namespace
