// astexpression.cpp
// this file is part of Context Free
// ---------------------
// Copyright (C) 2009-2013 John Horigan - john@glyphic.com
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// John Horigan can be contacted at john@glyphic.com or at
// John Horigan, 1209 Villa St., Mountain View, CA 94041-1123, USA
//
//


#include "astexpression.h"
#include "builder.h"
#include "rendererAST.h"

#include <math.h>
#include <typeinfo>
#include <cassert>

namespace AST {
    
    
    
    // Make sure that this stays in sync with the FuncType enum!
    const char* ASTfunction::FuncNames[ASTfunction::LastOne] = 
    {
        "cos", "sin", "tan", "cot", 
        "acos", "asin", "atan", "acot", 
        "cosh", "sinh", "tanh", 
        "acosh", "asinh", "atanh", 
        "log", "log10", "sqrt", "exp", "abs", "floor", "infinity", 
        "factorial", "sg", "isNatural",
        "bitnot", "bitor", "bitand", "bitxor", "bitleft", "bitright",
        "atan2", "mod", "divides", "div", "min", "max", "ftime", "frame",
        "rand_static", "rand", "rand+/-", "randint"
    };
    
    int ASTmodification::ModClass[ASTmodTerm::lastModType] = {  
        ASTmodification::NotAClass,                                 // unknown
        ASTmodification::GeomClass | ASTmodification::PathOpClass,  // x
        ASTmodification::GeomClass | ASTmodification::PathOpClass,  // y
        ASTmodification::ZClass,                                    // z
        ASTmodification::GeomClass | ASTmodification::ZClass,       // xyz
        ASTmodification::GeomClass,                                 // transform
        ASTmodification::GeomClass,                                 // size
        ASTmodification::GeomClass | ASTmodification::ZClass,       // sizexyz
        ASTmodification::GeomClass | ASTmodification::PathOpClass,  // rot
        ASTmodification::GeomClass,                                 // skew
        ASTmodification::GeomClass, ASTmodification::ZClass,        // flip, zsize
        ASTmodification::NotAClass,                                 // Entropy
        ASTmodification::HueClass, ASTmodification::SatClass,       // hue, ...
        ASTmodification::BrightClass, ASTmodification::AlphaClass, 
        ASTmodification::HueClass, ASTmodification::SatClass,       // hue|, ...
        ASTmodification::BrightClass, ASTmodification::AlphaClass, 
        ASTmodification::HueTargetClass, ASTmodification::SatTargetClass, // |hue, ...
        ASTmodification::BrightTargetClass, ASTmodification::AlphaTargetClass,
        ASTmodification::TimeClass, ASTmodification::TimeClass,     // time, timescale
        ASTmodification::StrokeClass, ASTmodification::ParamClass,  // stroke, param
        ASTmodification::PathOpClass, ASTmodification::PathOpClass, // x1, y1
        ASTmodification::PathOpClass, ASTmodification::PathOpClass, // x2, y2
        ASTmodification::PathOpClass, ASTmodification::PathOpClass, // xrad, yrad
        -1                                                          // modification
    };
    
    // These random strings are courtesy of http://www.fourmilab.ch/hotbits/
    const char* ASTmodTerm::Entropies[ASTmodTerm::lastModType] = {
        "",                                                     // unknown
        "\x95\xE7\x48\x5E\xCC\x06",                             // x
        "\x84\x2B\xF3\xBB\x93\x59",                             // y
        "\xC8\x3A\x12\x32\x36\x71",                             // z
        "\x6C\x31\xCA\xBF\x8D\x89",                             // xyz
        "\x88\x90\x54\xC5\xD3\x20",                             // transform
        "\x64\xEC\x5B\x4B\xEE\x2B",                             // size
        "\xB0\x31\xD5\x1E\x7A\x5A",                             // sizexyz
        "\x84\xB0\x92\x26\x59\xE2",                             // rot
        "\xFF\x2D\x84\x01\xA0\x0A",                             // skew
        "\x43\x5A\x17\xEA\x12\x05", "\x64\xEC\x5B\x4B\xEE\x2B", // flip, zsize
        "",                                                     // Entropy
        "\x02\xDE\x2B\x2C\x25\xA1", "\x18\x4F\xCF\x04\x3F\xE5", // hue, ...
        "\x1F\x3F\xEB\xA2\xA2\x7E", "\xB4\xFF\x9E\x45\xEE\x7E",
        "\xAF\xE5\x58\x33\x20\xF8", "\x98\x80\xED\x44\x2F\xF2", // hue|, ...
        "\x68\xD6\xCB\x8A\x96\x20", "\x24\x4C\xCC\x41\x09\xC7",
        "\xDB\x3F\xA1\xDA\xE7\x45", "\xDA\x75\x13\xD3\x30\xEA", // |hue, ...
        "\x8F\x01\x2B\x75\xC3\x25", "\xE7\xCD\x5E\xE3\x88\xF4",
        "\x20\xC6\xE8\x02\xED\x27", "\x78\x8E\xC8\x2C\x1C\x96", // time, timescale
        "", "",                                                 // stroke, param
        "", "",                                                 // x1, y1
        "", "",                                                 // x2, y2
        "", "",                                                 // xrad, yrad
        "\x88\x90\x54\xC5\xD3\x20"                              // modification
    };
    
    const char* ASTfunction::Entropies[ASTfunction::LastOne] = {
        "\xA1\xE7\x9C\x1A\xAF\x7D",     // Cos
        "\xAF\x58\xFE\x2C\xD4\x53",     // Sin
        "\x95\xFF\x59\x11\x03\x02",     // Tan
        "\x77\xF5\xB6\x35\x8C\xF0",     // Cot
        "\x3A\xCD\x79\x3E\xAD\xB4",     // Acos
        "\x1D\x75\x0B\xBC\x5F\x52",     // Asin
        "\x0B\xC8\x89\xAB\xF8\xB7",     // Atan
        "\x69\x7C\xC7\x1A\xF6\x7B",     // Acot
        "\x48\x43\x43\x35\x62\x81",     // Cosh
        "\x51\x62\xFB\x76\xED\x9C",     // Sinh
        "\xBB\x91\x54\xA9\x63\x84",     // Tanh
        "\x4F\x28\x48\x20\xB7\x5C",     // Acosh
        "\x6C\x9B\x32\xAA\x4C\xD0",     // Asinh
        "\x58\xEC\xBB\x25\xF8\xB6",     // Atanh
        "\x8E\xB8\x62\xA1\x75\x0F",     // Log
        "\x4A\x6C\xA3\x02\x8B\x80",     // Log10
        "\x86\x7C\xFC\x20\xCB\x97",     // Sqrt
        "\x88\xA8\x65\xF0\xC1\x06",     // Exp
        "\x41\x89\x18\xD1\xAD\x82",     // Abs
        "\xB7\x28\xD7\xD7\xA3\xCC",     // Floor
        "\x2C\x28\x50\xCC\xDE\x44",     // Infinity
        "\x19\xD7\x83\x29\x47\x99",     // Factorial
        "\xB7\x05\x28\xBA\xCD\x2E",     // Sg
        "\x49\xD6\xF8\x5B\x45\x59",     // IsNatural
        "\x79\x19\x1A\x9F\x4D\xA0",     // BitNot
        "\xF2\x77\xAB\x5C\x33\x43",     // BitOr
        "\xC3\x56\x9E\x75\xE0\x44",     // BitAnd
        "\xBB\xFA\x2B\xD2\x91\x55",     // BitXOR
        "\x91\x47\xE5\xE5\x0D\xAA",     // BitLeft
        "\xF1\xAB\x17\x00\xFA\xA5",     // BitRight
        "\x99\x1B\xC9\xE0\x3F\xA4",     // Atan2
        "\x78\x8E\xC8\x2C\x1C\x96",     // Divides
        "\x64\xEC\x5B\x4B\xEE\x2B",     // Div
        "\x0F\xE3\xFE\x5F\xBF\xBF",     // Mod
        "\xA2\x42\xA3\x49\xB1\x19",     // Min
        "\xD3\x55\x5C\x0D\xD8\x51",     // Max
        "\x4F\xBE\xA1\x06\x80\x06",     // Ftime
        "\x90\x70\x6A\xBB\xBA\xB0",     // Frame
        "\xC8\xF7\xE5\x3E\x05\xA3",     // Rand_Static
        "\xDA\x18\x5B\xE2\xDB\x79",     // Rand
        "\xDC\x8D\x09\x15\x8A\xC4",     // Rand2
        "\x48\x14\x4E\x27\x35\x2E"      // RandInt
    };
    
    ASTruleSpecifier ASTruleSpecifier::Zero;
    
    ASTfunction::ASTfunction(const std::string& func, exp_ptr args, Rand64& r,
                             const yy::location& nameLoc, const yy::location& argsLoc)
    : ASTexpression(nameLoc + argsLoc, true, false, NumericType), 
      functype(NotAFunction), arguments(nullptr)
    {
        if (&func == nullptr || func.empty()) {
            CfdgError::Error(nameLoc, "bad function call");
            return;
        }
        
        isConstant = args ? args->isConstant : true;
        isLocal = args ? args->isLocal : true;
        int argcount = args ? args->evaluate(nullptr, 0) : 0;
        
        functype = GetFuncType(func);
        
        if (functype == NotAFunction) {
            CfdgError::Error(nameLoc, "Unknown function");
            return;
        }
        
        if (functype == ASTfunction::Infinity && argcount == 0) {
            arguments.reset(new ASTreal(1.0, argsLoc));
            return;
        }
        
        if (functype == Ftime) {
            if (args)
                CfdgError::Error(argsLoc, "ftime() function takes no arguments");
            isConstant = false;
            isLocal = true;
            arguments.reset(new ASTreal(1.0, argsLoc));
            return;
        }
        
        if (functype == Frame) {
            if (args)
                CfdgError::Error(argsLoc, "frame() functions takes no arguments");
            isConstant = false;
            isLocal = false;
            arguments.reset(new ASTreal(1.0, argsLoc));
            return;
        }
        
        if (functype >= Rand_Static && functype <= RandInt) {
            if (functype == Rand_Static) {
                random = r.getDouble();
            } else {
                isConstant = false;
            }
            
            switch (argcount) {
                case 0:
                    args.reset(new ASTcons(new ASTreal(0.0, argsLoc), 
                                           new ASTreal(functype == RandInt ? 2.0 : 1.0, argsLoc)));
                    break;
                case 1:
                    args.reset(new ASTcons(new ASTreal(0.0, argsLoc), args.release()));
                    break;
                case 2:
                    break;
                default:
                    CfdgError::Error(argsLoc, "Illegal argument(s) for random function");
                    break;
            }
            
            if (!isConstant && functype == Rand_Static) {
                CfdgError::Error(argsLoc, "Argument(s) for rand_static() must be constant");
            }

            arguments = std::move(args);
            if (functype == RandInt)
                isNatural = arguments->isNatural;
            return;
        }
        
        if (functype == Abs) {
            if (argcount < 1 || argcount > 2) {
                CfdgError::Error(argsLoc, "function takes one or two arguments");
            }
        } else if (functype < BitOr) {
            if (argcount != 1) {
                CfdgError::Error(argsLoc, functype == ASTfunction::Infinity ? 
                               "function takes zero or one arguments" : 
                               "function takes one argument");
            }
        } else if (functype < Min) {
            if (argcount != 2) {
                CfdgError::Error(argsLoc, "function takes two arguments");
            }
        } else {
            if (argcount < 2) {
                CfdgError::Error(argsLoc, "function takes at least two arguments");
            }
        }
        arguments = std::move(args);
        if (functype == Mod || functype == Abs || functype == Min ||
            functype == Max || (functype >= BitNot && functype <= BitRight))
        {
            isNatural = arguments->isNatural;
        }
        if (functype == Factorial || functype == Sg || functype == IsNatural ||
            functype == Div || functype == Divides)
        {
            if (!arguments->isNatural)
                CfdgError::Error(arguments->where, "function is defined over natural numbers only");
            isNatural = true;
        }
    }
    
    ASTfunction::FuncType
    ASTfunction::GetFuncType(const std::string& func) 
    {
        for (int i = 0; i < LastOne; ++i)
            if (func.compare(FuncNames[i]) == 0) {
                return static_cast<FuncType>(i);
            }
        
        return NotAFunction;
    }
    
    ASTruleSpecifier::ASTruleSpecifier(int t, const std::string& name, exp_ptr args, 
                                       const yy::location& loc, const ASTparameters* types,
                                       const ASTparameters* parent)
    : ASTexpression(loc, !args || args->isConstant, false, RuleType),
      shapeType(t), entropyVal(name), argSource(DynamicArgs),
      arguments(args.release()), simpleRule(nullptr), mStackIndex(0), typeSignature(types)
    {
        if (types && types->empty()) {
            types = nullptr;
            typeSignature = nullptr;
        }
        if (parent && parent->empty())
            parent = nullptr;
        argSize = ASTparameter::CheckType(types, parent, arguments.get(), loc, true);
        if (argSize < 0) {
            argSource = NoArgs;
            return;
        }
            
        if (arguments && arguments->mType != AST::NoType) {
            arguments->entropy(entropyVal);
            if (arguments->isConstant) {
                const StackRule* simp = evalArgs();
                simp->mRefCount = UINT32_MAX;
                simpleRule = simp;
                argSource = SimpleArgs;
            }
        } else if (arguments && arguments->mType == AST::NoType) {
            argSource = ParentArgs;
        } else {
            argSource = NoArgs;
            simpleRule = StackRule::alloc(shapeType, 0, types);
            simpleRule->mRefCount = UINT32_MAX;
        }
    }
    
    ASTruleSpecifier::ASTruleSpecifier(const std::string& name, const yy::location& loc, 
                                       int stackIndex)
    : ASTexpression(loc, false, false, RuleType), shapeType(0), argSize(0), 
      entropyVal(name), argSource(StackArgs),
      arguments(nullptr), simpleRule(nullptr), mStackIndex(stackIndex), typeSignature(nullptr)
    {
    }
    
    ASTruleSpecifier::ASTruleSpecifier(const ASTruleSpecifier* r, 
                                       const std::string& name, 
                                       const yy::location& loc)
    : ASTexpression(loc, false, false, RuleType), shapeType(r->shapeType), argSize(r->argSize), 
      entropyVal(name), argSource(NoArgs), arguments(nullptr), simpleRule(nullptr), mStackIndex(0),
      typeSignature(r->typeSignature)
    {
        if (r->argSource == SimpleArgs) {
            StackRule* simp = StackRule::alloc(shapeType, argSize, nullptr);
            simp->mRefCount = UINT32_MAX;
            argSource = SimpleArgs;
            simpleRule = simp;
            if (argSize)
                for (int i = 1; i < argSize + 1; ++i)
                    simp[i] = r->simpleRule[i];
            return;
        }
        assert(r->argSource == NoArgs || Builder::CurrentBuilder->mWant2ndPass);   // only duplicate constant rule specs
    }
    
    ASTruleSpecifier::ASTruleSpecifier(ASTruleSpecifier&& r)
    : ASTexpression(r.where, r.isConstant, false, r.mType), shapeType(r.shapeType),
      argSize(r.argSize), entropyVal(r.entropyVal), argSource(r.argSource),
      arguments(std::move(r.arguments)), simpleRule(r.simpleRule), mStackIndex(r.mStackIndex),
      typeSignature(r.typeSignature)
    {
        r.simpleRule = nullptr;    // move semantics
    }
    
    ASTruleSpecifier::ASTruleSpecifier(exp_ptr args, const yy::location& loc)
    : ASTexpression(loc, false, false, RuleType), shapeType(-1),
      argSize(0), argSource(ShapeArgs), arguments(std::move(args)),
      simpleRule(nullptr), mStackIndex(0), typeSignature(nullptr)
    {
        assert(arguments);
    }
    
    const StackRule*
    ASTruleSpecifier::evalArgs(RendererAST* rti, const StackRule* parent) const
    {
        switch (argSource) {
        case NoArgs:
        case SimpleArgs:
            return simpleRule;
        case StackArgs: {
            const StackType* stackItem = (mStackIndex < 0) ? rti->mLogicalStackTop + mStackIndex :
                                                             rti->mCFstack.data() + mStackIndex;
            stackItem->rule->retain(rti);
            return stackItem->rule;
        }
        case ParentArgs:
            assert(parent);
            assert(rti);
            if (shapeType != parent->mRuleName) {
                // Child shape is different from parent, even though parameters are reused,
                // and we can't finesse it in ASTreplacement::traverse(). Just
                // copy the parameters with the correct shape type.
                StackRule* ret = StackRule::alloc(shapeType, argSize, typeSignature);
                if (argSize)
                    for (int i = 1; i < argSize + 1; ++i)
                        ret[i] = parent[i];
                return ret;
            }
        case SimpleParentArgs:
            assert(parent);
            assert(rti);
            parent->retain(rti);
            return parent;
        case DynamicArgs: {
            StackRule* ret = StackRule::alloc(shapeType, argSize, typeSignature);
            ret->evalArgs(rti, arguments.get(), parent);
            return ret;
        }
        case ShapeArgs:
            return arguments->evalArgs(rti, parent);
        default:
            assert(false);
            return nullptr;
        }
    }
    
    const StackRule*
    ASTparen::evalArgs(RendererAST* rti, const StackRule* parent) const
    {
        if (mType != RuleType) {
            CfdgError::Error(where, "Evaluation of a non-shape expression in a shape context");
            return nullptr;
        }
        
        return e->evalArgs(rti, parent);
    }
    
    const StackRule*
    ASTselect::evalArgs(RendererAST* rti, const StackRule* parent) const
    {
        if (mType != RuleType) {
            CfdgError::Error(where, "Evaluation of a non-shape select() in a shape context");
            return nullptr;
        }
        
        return (*arguments)[getIndex(rti)]->evalArgs(rti, parent);
    }
    
    const StackRule*
    ASTuserFunction::evalArgs(RendererAST* rti, const StackRule* parent) const
    {
        if (mType != RuleType) {
            CfdgError::Error(where, "Function does not evaluate to a shape");
            return nullptr;
        }
        
        if (!rti)
            throw DeferUntilRuntime();
        
        if (rti->requestStop || Renderer::AbortEverything)
            throw CfdgError(where, "Stopping");
        
        const StackRule* ret = nullptr;
        
        if (definition->mStackCount) {
            size_t size = rti->mCFstack.size();
            if (size + definition->mStackCount > rti->mCFstack.capacity())
                CfdgError::Error(where, "Maximum stack size exceeded");
            const StackType*  oldLogicalStackTop = rti->mLogicalStackTop;
            rti->mCFstack.resize(size + definition->mStackCount);
            rti->mCFstack[size].evalArgs(rti, arguments.get(), &(definition->mParameters), isLet);
            rti->mLogicalStackTop = rti->mCFstack.data() + rti->mCFstack.size();
            ret = definition->mExpression->evalArgs(rti, parent);
            rti->mCFstack.resize(size);
            rti->mLogicalStackTop = oldLogicalStackTop;
        } else {
            ret = definition->mExpression->evalArgs(rti, parent);
        }
        return ret;
    }
    
    ASTcons::ASTcons(ASTexpression* l, ASTexpression* r)
    : ASTexpression(l->where, l->isConstant, l->isNatural, l->mType)
    {
        isLocal = l->isLocal;
        children.emplace_back(l);
        append(r);
    };

    ASTexpression*
    ASToperator::Op(char op, ASTexpression* l, ASTexpression* r)
    {
        if (l && r) return new ASToperator(op, l, r);
        return l ? l : r;
    }
    
    ASToperator::ASToperator(char o, ASTexpression* l, ASTexpression* r)
    : ASTexpression(r ? (l->where + r->where) : l->where), op(o), left(l), right(r) 
    {
        isConstant = r ? r->isConstant && l->isConstant : l->isConstant;
        isLocal = r ? r->isLocal && l->isLocal : l->isLocal;
        mType = r ? static_cast<expType>(l->mType | r->mType) : l->mType;
        if (strchr("+_*<>LG=n&|X^!", o))
            isNatural = r ? (l->isNatural && r->isNatural) : l->isNatural;
    }
    
    ASTmodTerm::ASTmodTerm(ASTmodTerm::modTypeEnum t, ASTexpression* a, const yy::location& loc)
    : ASTexpression(loc, a->isConstant, false, ModType), modType(t), args(a)
    {
        if (a->mType == AST::RuleType)
            CfdgError::Error(loc, "Illegal expression in shape adjustment");
        
        if (a->mType == AST::ModType) {
            if (t != ASTmodTerm::transform)
                CfdgError::Error(loc, "Cannot accept a transform expression here");

            modType = ASTmodTerm::modification;
        }
    }
    
    ASTmodification::ASTmodification(const ASTmodification& m, const yy::location& loc)
    : ASTexpression(loc, true, false, ModType), modData(m.modData), 
      modClass(m.modClass), strokeWidth(m.strokeWidth), flags(m.flags), 
      entropyIndex(m.entropyIndex)
    {
        assert(m.modExp.empty());
    }
    
    ASTmodification::ASTmodification(mod_ptr m, const yy::location& loc)
    : ASTexpression(loc, true, false, ModType), entropyIndex(0)
    {
        if (m) {
            modData.mRand64Seed.seed(0);
            grab(m.get());
        } else {
            modClass = 0;
            strokeWidth = 0.1;
            flags = CF_MITER_JOIN + CF_BUTT_CAP + CF_FILL;
        }
    }
    
    void
    ASTmodification::grab(AST::ASTmodification* m)
    {
        Rand64 oldEntropy = modData.mRand64Seed;
        modData = m->modData;
        modData.mRand64Seed ^= oldEntropy;
        modExp.swap(m->modExp);
        modClass = m->modClass;
        strokeWidth = m->strokeWidth;
        flags = m->flags;
        entropyIndex = (entropyIndex + m->entropyIndex) & 7;
        isConstant = modExp.empty();
    }
    
    ASTselect::ASTselect(exp_ptr args, const yy::location& loc, bool asIf)
    : ASTexpression(loc), tupleSize(-1), indexCache(0), arguments(std::move(args)),
      ifSelect(asIf)
    {
        isConstant = false;
        arguments->entropy(ent);
        ent.append("\xB5\xA2\x4A\x74\xA9\xDF");
        
        if (arguments->size() < 3) {
            CfdgError::Error(loc, "select()/if() function requires arguments");
            return;
        }
        
        if ((*arguments)[0]->mType != NumericType || 
            (*arguments)[0]->evaluate(nullptr, 0) != 1)
        {
            CfdgError::Error((*arguments)[0]->where, "is()/select() selector must be a numeric scalar");
            return;
        }
        
        mType = (*arguments)[1]->mType;
        isLocal = (*arguments)[0]->isLocal && (*arguments)[1]->isLocal;
        isNatural = (*arguments)[1]->isNatural;
        tupleSize = (mType == NumericType) ? (*arguments)[1]->evaluate(nullptr, 0) : 1;
        if (tupleSize > 1) isNatural = false;
        if (tupleSize == -1)
            CfdgError::Error((*arguments)[1]->where, "Error determining tuple size");
        
        for (int i = 2; i < arguments->size(); ++i) {
            if (mType != (*arguments)[i]->mType) {
                CfdgError::Error((*arguments)[i]->where, "select()/if() choices must be of same type");
            } else if (mType == NumericType && tupleSize != -1 && 
                     (*arguments)[i]->evaluate(nullptr, 0) != tupleSize)
            {
                CfdgError::Error((*arguments)[i]->where, "select()/if() choices must be of same length");
            }
            isLocal = isLocal && (*arguments)[i]->isLocal;
            isNatural = isLocal && (*arguments)[i]->isNatural;
        }

        if (ifSelect && arguments->size() != 3) {
            CfdgError::Error(loc, "if() function requires two arguments");
        }
        
        if ((*arguments)[0]->isConstant) {
            indexCache = getIndex();
            isConstant = (*arguments)[indexCache]->isConstant;
            isLocal = (*arguments)[indexCache]->isLocal;
            isNatural = (*arguments)[indexCache]->isNatural;
        }
    }
    
    ASTuserFunction::ASTuserFunction(ASTexpression* args, ASTdefine* func, 
                                     const yy::location& nameLoc)
    : ASTexpression(args ? (nameLoc + args->where) : nameLoc, 
                    false, false, func->mType),
      definition(func), arguments(args), isLet(false)
    {
        if (definition->mExpression) {
            isConstant = isConstant && definition->mExpression->isConstant;
            isNatural = definition->mExpression->isNatural;
        } else {
            isConstant = isConstant && definition->mChildChange.modExp.empty();
        }
        isLocal = args ? args->isLocal : true;
        if (args && !func->mStackCount)
            CfdgError::Error(nameLoc + args->where, "Function does not take arguments");
        if (!args && func->mStackCount)
            CfdgError::Error(nameLoc, "Function takes arguments");
        if (args && func->mStackCount)
            ASTparameter::CheckType(&(func->mParameters), nullptr, args, args->where, false);
    }
    
    ASTlet::ASTlet(ASTexpression* args, ASTdefine* func, const yy::location& letLoc,
                   const yy::location& defLoc)
    : AST::ASTuserFunction(args, func, letLoc)
    {
        where = where + defLoc;
        isLet = true;
    }
    
    ASTarray::ASTarray(const ASTparameter* bound, exp_ptr args, int stackOffset,
                       const yy::location& loc, const std::string& name)
    : ASTexpression(loc, bound->mStackIndex == -1, bound->isNatural, bound->mType),
      mConstData(bound->mStackIndex == -1), mArgs(nullptr), mLength(1), mStride(1),
      mStackIndex(bound->mStackIndex - stackOffset), 
      mCount(bound->mType == NumericType ? bound->mTuplesize : 1),
      isParameter(bound->isParameter), entString(name)
    {
        if (!args || args->mType != AST::NumericType) {
            CfdgError::Error(loc, "Array arguments must be numeric");
            mArgs.reset(new ASTreal(0.0, loc));
            isConstant = mConstData = false;
            return;     // deleting args
        }

        isLocal = bound->isLocal;
        args->entropy(entString);
        if (mConstData) {
            mConstData = bound->mDefinition->mExpression->evaluate(mData, 9) > 0;
        }
        if ((*args)[0]->evaluate(nullptr, 0) == 1) {
            mArgs.reset((*args)[0]);
            if (!args->release(0)) {
                args.release();
                args.reset(new ASTexpression(mArgs->where)); // replace with dummy
            }
            double data[2];
            int count = 0;
            for (int i = 1; i < args->size(); ++i) {
                if (!(*args)[i]->isConstant) {
                    CfdgError::Error((*args)[i]->where, "Array argument is not constant");
                    break;
                }
                int num = (*args)[i]->evaluate(data + count, 2 - count);
                if (num <= 0) {
                    CfdgError::Error((*args)[i]->where, "Error evaluating array arguments");
                    break;
                }
                count += num;
            }
            switch (count) {
                case 2:
                    mStride = static_cast<int>(data[1]);  // fall through
                case 1:
                    mLength = static_cast<int>(data[0]);  // fall through
                case 0:
                    break;
                    
                default:
                    CfdgError::Error(args->where, "Unexpected number of array arguments");
                    break;
            }
        } else if (args->isConstant) {
            double data[3];
            switch (args->evaluate(data, 3)) {
                case 3:
                    mStride = static_cast<int>(data[2]);
                    // fall through
                case 2:
                    mLength = static_cast<int>(data[1]);
                    // fall through
                case 1:
                    mArgs.reset(new ASTreal(data[0], args->where));
                    mArgs->isLocal = args->isLocal;
                    break;
                    
                default:
                    CfdgError::Error(args->where, "Error evaluating array arguments");
                    break;
            }
        } else {
            mArgs = std::move(args);
            if (mArgs->evaluate(nullptr, 0) != 1)
                CfdgError::Error(mArgs->where, "Array length & stride arguments must be contant");
        }
        if (mStride < 0 || mLength < 0)
            CfdgError::Error(mArgs->where, "Array length & stride arguments must be positive");
        if (mStride * (mLength - 1) >= mCount)
            CfdgError::Error(mArgs->where, "Array length & stride arguments too large for source");
        isConstant = isConstant && mArgs->isConstant;
        isLocal = isLocal && mArgs->isLocal;
    }

    ASTruleSpecifier::~ASTruleSpecifier()
    {
        if (simpleRule) {
            simpleRule->mRefCount = 0;
            simpleRule->release();
        }
    };
    
    ASTcons::~ASTcons()
    {
    }
    
    ASTselect::~ASTselect()
    {
    }
    
    ASTmodification::~ASTmodification()
    {
    }
    
    ASTarray::~ASTarray()
    {
    }
    
    ASTlet::~ASTlet()
    {
        delete definition;
    }

    static void
    Setmod(term_ptr& mod, term_ptr& newmod)
    {
        if (mod)
            CfdgError::Warning(mod->where, "Warning: this term is being dropped");
        mod = std::move(newmod);
    }
    
    void
    ASTmodification::makeCanonical()
    // Receive a vector of modification terms and return an ASTexpression with
    // those terms rearranged into TRSSF canonical order. Duplicate terms are
    // deleted with a warning.
    {
        ASTtermArray temp;
        temp.swap(modExp);
        
        {   // no need for try/catch block to clean up temp array
            term_ptr x;
            term_ptr y;
            term_ptr z;
            term_ptr rot;
            term_ptr skew;
            term_ptr size;
            term_ptr zsize;
            term_ptr flip;
            term_ptr xform;
            
            for (term_ptr& mod: temp) {
                assert(mod);
                
                int argcount = 0;
                if (mod->args && mod->args->mType == NumericType)
                    argcount = mod->args->evaluate(nullptr, 0);
                
                switch (mod->modType) {
                    case ASTmodTerm::x:
                        Setmod(x, mod);
                        if (argcount > 1) {
                            y.reset();
                        }
                        break;
                    case ASTmodTerm::y:
                        Setmod(y, mod);
                        break;
                    case ASTmodTerm::z:
                        Setmod(z, mod);
                        break;
                    case ASTmodTerm::modification:
                    case ASTmodTerm::transform:
                        Setmod(xform, mod);
                        break;
                    case ASTmodTerm::rot:
                        Setmod(rot, mod);
                        break;
                    case ASTmodTerm::size:
                        Setmod(size, mod);
                        break;
                    case ASTmodTerm::zsize:
                        Setmod(zsize, mod);
                        break;
                    case ASTmodTerm::skew:
                        Setmod(skew, mod);
                        break;
                    case ASTmodTerm::flip:
                        Setmod(flip, mod);
                        break;
                    default:
                        modExp.push_back(std::move(mod));
                        break;
                }
            }
            
            temp.clear();
            
            // If x and y are provided then merge them into a single (x,y) modification
            if (x && y && x->args->evaluate(nullptr, 0) == 1 && y->args->evaluate(nullptr, 0) == 1) {
                x->args.reset(x->args.release()->append(y->args.release()));
                y.reset();
            }
            
            if (    x) modExp.push_back(std::move(x));
            if (    y) modExp.push_back(std::move(y));
            if (    z) modExp.push_back(std::move(z));
            if (  rot) modExp.push_back(std::move(rot));
            if ( size) modExp.push_back(std::move(size));
            if (zsize) modExp.push_back(std::move(zsize));
            if ( skew) modExp.push_back(std::move(skew));
            if ( flip) modExp.push_back(std::move(flip));
            if (xform) modExp.push_back(std::move(xform));
        }
    }
    
    ASTexpression*
    ASTexpression::Append(ASTexpression* l, ASTexpression* r)
    {
        if (l && r) return l->append(r);
        return l ? l : r;
    }
    
    ASTexpression*
    ASTexpression::append(AST::ASTexpression *sib)
    {
        return sib ? new ASTcons(this, sib) : this;
    }
    
    ASTexpression*
    ASTcons::append(AST::ASTexpression *sib)
    {
        if (!sib) return this;
        where = where + sib->where;
        isConstant = isConstant && sib->isConstant;
        isNatural = isNatural && sib->isNatural;
        isLocal = isLocal && sib->isLocal;
        mType = static_cast<expType>(mType | sib->mType);
        
        // Cannot insert an ASTcons into children, it will be flattened away.
        // You must wrap the ASTcons in an ASTparen in order to insert it whole.
        for (int i = 0; i < sib->size(); ++i)
            children.emplace_back((*sib)[i]);
        if (sib->release())
            delete sib;
        return this;
    }
    
    bool
    ASTcons::release(size_t i)
    {
        if (i == std::numeric_limits<size_t>::max()) {
            for (exp_ptr& child: children) {
                child.release();
            }
            children.clear();
        } else if (i < children.size()) {
            children[i].release();
        } else {
            CfdgError::Error(where, "Expression list bounds exceeded");
        }
        return true;
    }
    
    ASTexpression*
    ASTexpression::operator[](size_t i)
    {
        if (i)
            CfdgError::Error(where, "Expression list bounds exceeded");
        return this;
    }
    
    const ASTexpression*
    ASTexpression::operator[](size_t i) const
    {
        if (i)
            CfdgError::Error(where, "Expression list bounds exceeded");
        return this;
    }
    
    ASTexpression*
    ASTcons::operator[](size_t i)
    {
        if (i >= children.size()) {
            CfdgError::Error(where, "Expression list bounds exceeded");
            return this;
        }
        return children[i].get();
    }
    
    const ASTexpression*
    ASTcons::operator[](size_t i) const
    {
        if (i >= children.size()) {
            CfdgError::Error(where, "Expression list bounds exceeded");
            return this;
        }
        return children[i].get();
    }
    
    // Evaluate a cons tree to see how many reals it has and optionally
    // copy them to an array
    int
    ASTcons::evaluate(double* res, int length, RendererAST* rti) const
    {
        if ((static_cast<int>(mType) & (NumericType | FlagType)) == 0 ||
            (static_cast<int>(mType) & (ModType | RuleType)))
        {
            CfdgError::Error(where, "Non-numeric expression in a numeric context");
            return -1;
        }
        
        int count = 0;
        for (size_t i = 0; i < children.size(); ++i) {
            int num = children[i]->evaluate(res, length, rti);
            if (num <= 0)
                return -1;
            count += num;
            if (res) {
                res += num;
                length -= num;
            }
        }
        
        return count;
    }
    
    int
    ASTreal::evaluate(double* res, int length, RendererAST*) const
    {
        if (res && length < 1)
            return -1;
        if (res) 
            *res = value;
        return 1;
    }
    
    int
    ASTvariable::evaluate(double* res, int length, RendererAST* rti) const
    {
        if (mType != NumericType) {
            CfdgError::Error(where, "Non-numeric variable in a numeric context");
            return -1;
        }
        if (res && (length < count))
            return -1;
        if (res) {
            if (rti == nullptr) throw DeferUntilRuntime();
            const StackType* stackItem = (stackIndex < 0) ? rti->mLogicalStackTop + stackIndex :
                                                            rti->mCFstack.data() + stackIndex;
            for (int i = 0; i < count; ++i)
                res[i] = stackItem[i].number;
        }
        
        return count;
    }
    
    int
    ASTuserFunction::evaluate(double* res, int length, RendererAST* rti) const
    {
        if (mType != NumericType) {
            CfdgError::Error(where, "Function does not evaluate to a number");
            return -1;
        }
        if (res && length < definition->mTuplesize)
            return -1;
        if (!res)
            return definition->mTuplesize;
        if (!rti)
            throw DeferUntilRuntime();
        
        if (rti->requestStop || Renderer::AbortEverything)
            throw CfdgError(where, "Stopping");
        
        if (definition->mStackCount) {
            size_t size = rti->mCFstack.size();
            if (size + definition->mStackCount > rti->mCFstack.capacity())
                CfdgError::Error(where, "Maximum stack size exceeded");
            const StackType*  oldLogicalStackTop = rti->mLogicalStackTop;
            rti->mCFstack.resize(size + definition->mStackCount);
            rti->mCFstack[size].evalArgs(rti, arguments.get(), &(definition->mParameters), isLet);
            rti->mLogicalStackTop = rti->mCFstack.data() + rti->mCFstack.size();
            definition->mExpression->evaluate(res, length, rti);
            rti->mCFstack.resize(size);
            rti->mLogicalStackTop = oldLogicalStackTop;
        } else {
            definition->mExpression->evaluate(res, length, rti);
        }
        return definition->mTuplesize;
    }
    
    int
    ASToperator::evaluate(double* res, int length, RendererAST* rti) const
    {
        double l = 0.0;
        double r = 0.0;
        
        if (res && length < 1)
            return -1;
        
        if (mType == FlagType && op == '+') {
            if (left->evaluate(res ? &l : nullptr, 1, rti) != 1)
                return -1;
            if (!right || right->evaluate(res ? &r : nullptr, 1, rti) != 1)
                return -1;
            int f = static_cast<int>(l) | static_cast<int>(r);
            if (res)
                *res = static_cast<double>(f);
            return 1;
        }
        
        if (mType != NumericType) {
            CfdgError::Error(where, "Non-numeric expression in a numeric context");
            return -1;
        }
        
        if (left->evaluate(res ? &l : nullptr, 1, rti) != 1) {
            CfdgError::Error(left->where, "illegal operand");
            return -1;
        }
        
        // short-circuit evaluate && and ||
        if (res && (op == '&' || op == '|')) {
            if (l != 0.0 && op == '|') {
                *res = l;
                return 1;
            }
            if (l == 0.0 && op == '&') {
                *res = 0.0;
                return 1;
            }
        }
        
        int rightnum = right ? right->evaluate(res ? &r : nullptr, 1, rti) : 0;
        
        if (rightnum == 0 && (op == 'N' || op == 'P' || op == '!')) {
            if (res) {
                switch (op) {
                    case 'P':
                        *res = l;
                        break;
                    case 'N':
                        *res = -l;
                        break;
                    case '!':
                        *res = (l == 0.0) ? 1.0 : 0.0;
                        break;
                    default:
                        return -1;
                }
            }
            return 1;
        }
        
        if (rightnum != 1) {
            CfdgError::Error(left->where, "illegal operand");
            return -1;
        }
        
        
        if (res) {
            switch(op) {
                case '+':
                    *res = l + r;
                    break;
                case '-':
                    *res = l - r;
                    break;
                case '_':
                    *res = l - r > 0.0 ? l - r : 0.0;
                    break;
                case '*':
                    *res = l * r;
                    break;
                case '/':
                    *res = l / r;
                    break;
                case '<':
                    *res = (l < r) ? 1.0 : 0.0;
                    break;
                case 'L':
                    *res = (l <= r) ? 1.0 : 0.0;
                    break;
                case '>':
                    *res = (l > r) ? 1.0 : 0.0;
                    break;
                case 'G':
                    *res = (l >= r) ? 1.0 : 0.0;
                    break;
                case '=':
                    *res = (l == r) ? 1.0 : 0.0;
                    break;
                case 'n':
                    *res = (l != r) ? 1.0 : 0.0;
                    break;
                case '&':
                case '|':
                    *res = r;
                    break;
                case 'X':
                    *res = ((l && !r) || (!l && r)) ? 1.0 : 0.0;
                    break;
                case '^':
                    *res = pow(l, r);
                    if (isNatural && *res < 9007199254740992.) {
                        uint64_t pow = 1;
                        uint64_t il = static_cast<uint64_t>(l);
                        uint64_t ir = static_cast<uint64_t>(r);
                        while (ir) {
                            if (ir & 1) pow *= il;
                            il *= il;
                            ir >>= 1;
                        }
                        *res = static_cast<double>(pow);
                    }
                    break;
                default:
                    return -1;
            }
        } else {
            if (strchr("+-*/^_<>LG=n&|X", op) == nullptr)
                return -1;
        }
        
        return 1;
    }
    
    static double MinMax(const ASTexpression* e, RendererAST* rti, bool isMin)
    {
        double res = 0.0;
        if ((*e)[0]->evaluate(&res, 1, rti) != 1)
            CfdgError::Error((*e)[0]->where, "Error computing min/max here.");
        for (int i = 1; i < e->size(); ++i) {
            double v;
            if ((*e)[i]->evaluate(&v, 1, rti) != 1)
                CfdgError::Error((*e)[i]->where, "Error computing min/max here.");
            bool leftMin = res < v;
            res = ((isMin && leftMin) || (!isMin && !leftMin)) ? res : v;
        }
        return res;
    }
    
    int 
    ASTfunction::evaluate(double* res, int length, RendererAST* rti) const
    {
        if (mType != NumericType) {
            CfdgError::Error(where, "Non-numeric expression in a numeric context");
            return -1;
        }
        if ((res && length < 1) || (functype <= NotAFunction) || (functype >= LastOne))
            return -1;
        
        if (!res)
            return 1;
        
        if (functype == Min || functype == Max) {
            *res = MinMax(arguments.get(), rti, functype == Min);
            return 1;
        }
        
        double a[2];
        int count = arguments->evaluate(a, 2, rti);
        // no need to check the argument count, the constructor already checked it
        
        // But check it anyway to make valgrind happy
        if (count < 0) return 1;

        switch (functype) {
            case  Cos:  
                *res = cos(a[0] * 0.0174532925199);
                break;
            case  Sin:  
                *res = sin(a[0] * 0.0174532925199);
                break;
            case  Tan:  
                *res = tan(a[0] * 0.0174532925199);
                break;
            case  Cot:  
                *res = 1.0 / tan(a[0] * 0.0174532925199);
                break;
            case  Acos:  
                *res = acos(a[0]) * 57.29577951308;
                break;
            case  Asin:  
                *res = asin(a[0]) * 57.29577951308;
                break;
            case  Atan:  
                *res = atan(a[0]) * 57.29577951308;
                break;
            case  Acot:  
                *res = atan(1.0 / a[0]) * 57.29577951308;
                break;
            case  Cosh:  
                *res = cosh(a[0]);
                break;
            case  Sinh:  
                *res = sinh(a[0]);
                break;
            case Tanh:  
                *res = tanh(a[0]);
                break;
            case Acosh:  
                *res = acosh(a[0]);
                break;
            case Asinh:  
                *res = asinh(a[0]);
                break;
            case Atanh:  
                *res = atanh(a[0]);
                break;
            case Log:  
                *res = log(a[0]);
                break;
            case Log10:  
                *res = log10(a[0]);
                break;
            case Sqrt:  
                *res = sqrt(a[0]);
                break;
            case Exp:  
                *res = exp(a[0]);
                break;
            case Abs:  
                if (count == 1)
                    *res = fabs(a[0]);
                else 
                    *res = fabs(a[0] - a[1]);
                break;
            case Infinity:
                *res = (a[0] < 0.0) ? (-Renderer::Infinity) : (Renderer::Infinity);
                break;
            case Factorial:
                if (a[0] < 0.0 || a[0] > 18.0 ||a[0] != floor(a[0]))
                    CfdgError::Error(this->where, "Illegal argument for factorial");
                *res = 1.0;
                for (double v = 1.0; v <= a[0]; v += 1.0) *res *= v;
                break;
            case Sg:
                *res = a[0] == 0.0 ? 0.0 : 1.0;
                break;
            case IsNatural:
                *res = RendererAST::isNatural(rti, a[0]);
                break;
            case BitNot:
                *res = static_cast<double>(~static_cast<uint64_t>(a[0]) & 0xfffffffffffffull);
                break;
            case BitOr:
                *res = static_cast<double>((static_cast<uint64_t>(a[0]) | static_cast<uint64_t>(a[1])) & 0xfffffffffffffull);
                break;
            case BitAnd:
                *res = static_cast<double>((static_cast<uint64_t>(a[0]) & static_cast<uint64_t>(a[1])) & 0xfffffffffffffull);
                break;
            case BitXOR:
                *res = static_cast<double>((static_cast<uint64_t>(a[0]) ^ static_cast<uint64_t>(a[1])) & 0xfffffffffffffull);
                break;
            case BitLeft:
                *res = static_cast<double>((static_cast<uint64_t>(a[0]) << static_cast<uint64_t>(a[1])) & 0xfffffffffffffull);
                break;
            case BitRight:
                *res = static_cast<double>((static_cast<uint64_t>(a[0]) >> static_cast<uint64_t>(a[1])) & 0xfffffffffffffull);
                break;
            case Atan2: 
                *res = atan2(a[0], a[1]) * 57.29577951308;
                break;
            case Mod: 
                if (arguments->isNatural)
                    *res = static_cast<double>(static_cast<uint64_t>(a[0]) % static_cast<uint64_t>(a[1]));
                else
                    *res = fmod(a[0], a[1]);
                break;
            case Divides:
                *res = (static_cast<uint64_t>(a[0]) % static_cast<uint64_t>(a[1]) == 0ULL) ? 1.0 : 0.0;
                break;
            case Div:
                *res = static_cast<double>(static_cast<uint64_t>(a[0]) / static_cast<uint64_t>(a[1]));
                break;
            case Floor:
                *res = floor(a[0]);
                break;
            case Ftime:
                if (rti == nullptr) throw DeferUntilRuntime();
                *res = rti->mCurrentTime;
                break;
            case Frame:
                if (rti == nullptr) throw DeferUntilRuntime();
                *res = rti->mCurrentFrame;
                break;
            case Rand_Static: 
                *res = random * fabs(a[1] - a[0]) + fmin(a[0], a[1]);
                break;
            case Rand: 
                if (rti == nullptr) throw DeferUntilRuntime();
                rti->mRandUsed = true;
                *res = rti->mCurrentSeed.getDouble() * fabs(a[1] - a[0]) + fmin(a[0], a[1]);
                break;
            case Rand2: 
                if (rti == nullptr) throw DeferUntilRuntime();
                rti->mRandUsed = true;
                *res = (rti->mCurrentSeed.getDouble() * 2.0 - 1.0) * a[1] + a[0];
                break;
            case RandInt: 
                if (rti == nullptr) throw DeferUntilRuntime();
                rti->mRandUsed = true;
                *res = floor(rti->mCurrentSeed.getDouble() * fabs(a[1] - a[0]) + fmin(a[0], a[1]));
                break;
            default:
                return -1;
        }
        
        return 1;
    }
    
    int 
    ASTselect::evaluate(double* res, int length, RendererAST* rti) const
    {
        if (mType != NumericType) {
            CfdgError::Error(where, "Evaluation of a non-numeric select() in a numeric context");
            return -1;
        }
        
        if (res == nullptr)
            return tupleSize;
        
        return (*arguments)[getIndex(rti)]->evaluate(res, length, rti);
    }
    
    int
    ASTruleSpecifier::evaluate(double* , int , RendererAST* ) const
    {
        CfdgError::Error(where, "Improper evaluation of a rule specifier");
        return -1;
    }
    
    int 
    ASTparen::evaluate(double* res, int length, RendererAST* rti) const
    {
        if (mType != NumericType) {
            CfdgError::Error(where, "Non-numeric/flag expression in a numeric/flag context");
            return -1;
        }
        return e->evaluate(res, length, rti);
    }
    
    int
    ASTmodTerm::evaluate(double* , int , RendererAST* ) const
    {
        CfdgError::Error(where, "Improper evaluation of an adjustment expression");
        return -1;
    }
    
    int
    ASTmodification::evaluate(double* , int , RendererAST* ) const
    {
        CfdgError::Error(where, "Improper evaluation of an adjustment expression");
        return -1;
    }
    
    int
    ASTarray::evaluate(double* res, int length, RendererAST* rti) const
    {
        if (mType != NumericType) {
            CfdgError::Error(where, "Non-numeric/flag expression in a numeric/flag context");
            return -1;
        }
        if (res && (length < mLength))
            return -1;
        
        if (res) {
            if (rti == nullptr && !mConstData) throw DeferUntilRuntime();
            
            double i;
            if (mArgs->evaluate(&i, 1, rti) != 1) {
                CfdgError::Error(mArgs->where, "Cannot evaluate array index");
                return -1;
            }
            int index = static_cast<int>(i);
            if ((mLength - 1) * mStride + index >= mCount || index < 0) {
                CfdgError::Error(where, "array index exceeds bounds");
                return -1;
            }
            
            const double* source = mData;
            if (!mConstData)
                source = (mStackIndex < 0) ? &(rti->mLogicalStackTop[mStackIndex].number) :
                                             &(rti->mCFstack[mStackIndex].number);
            
            for (int i = 0; i < mLength; ++i)
                res[i] = source[i * mStride + index];
        }
        
        return mLength;
    }
    
    void
    ASTselect::evaluate(Modification& m, int* p, double* width, 
                        bool justCheck, int& seedIndex, bool shapeDest,
                        RendererAST* rti) const
    {
        if (mType != ModType) {
            CfdgError::Error(where, "Evaluation of a non-adjustment select() in an adjustment context");
            return;
        }
        
        (*arguments)[getIndex(rti)]->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
    }
    
    void
    ASTvariable::evaluate(Modification& m, int*, double*, 
                          bool justCheck, int&, bool shapeDest,
                          RendererAST* rti) const
    {
        if (mType != ModType)
            CfdgError::Error(where, "Non-adjustment variable referenced in an adjustment context");
        
        if (rti == nullptr) throw DeferUntilRuntime();
        if (justCheck) return;
        const StackType* stackItem = (stackIndex < 0) ? rti->mLogicalStackTop + stackIndex :
                                                        rti->mCFstack.data() + stackIndex;
        const Modification* smod = reinterpret_cast<const Modification*> (stackItem);
        if (shapeDest) {
            m *= *smod;
        } else {
            if (m.merge(*smod))
                RendererAST::ColorConflict(rti, where);
        }
    }
    
    void
    ASTcons::evaluate(Modification& m, int* p, double* width, 
                      bool justCheck, int& seedIndex, bool shapeDest,
                      RendererAST* rti) const
    {
        for (size_t i = 0; i < children.size(); ++i)
            children[i]->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
    }
    
    void
    ASTuserFunction::evaluate(Modification &m, int *p, double *width, 
                              bool justCheck, int &seedIndex, bool shapeDest,
                              RendererAST* rti) const
    {
        if (mType != ModType) {
            CfdgError::Error(where, "Function does not evaluate to an adjustment");
            return;
        }
        
        if (!rti)
            throw DeferUntilRuntime();
        
        if (rti->requestStop || Renderer::AbortEverything)
            throw CfdgError(where, "Stopping");
        
        if (definition->mStackCount) {
            size_t size = rti->mCFstack.size();
            if (size + definition->mStackCount > rti->mCFstack.capacity())
                CfdgError::Error(where, "Maximum stack size exceeded");
            const StackType*  oldLogicalStackTop = rti->mLogicalStackTop;
            rti->mCFstack.resize(size + definition->mStackCount);
            rti->mCFstack[size].evalArgs(rti, arguments.get(), &(definition->mParameters), isLet);
            rti->mLogicalStackTop = rti->mCFstack.data() + rti->mCFstack.size();
            definition->mExpression->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
            rti->mCFstack.resize(size);
            rti->mLogicalStackTop = oldLogicalStackTop;
        } else {
            definition->mExpression->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
        }
    }
    
    void
    ASTmodification::evaluate(Modification& m, int* p, double* width, 
                              bool justCheck, int& seedIndex, bool shapeDest,
                              RendererAST* rti) const
    {
        if (shapeDest) {
            m *= modData;
        } else {
            if (m.merge(modData))
                RendererAST::ColorConflict(rti, where);
        }
        
        for (const term_ptr& term: modExp)
            term->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
    }
    
    void
    ASTmodification::setVal(Modification& m, int* p, double* width, 
                            bool justCheck, int& seedIndex, 
                            RendererAST* rti) const
    {
        m = modData;
        for (const term_ptr& term: modExp)
            term->evaluate(m, p, width, justCheck, seedIndex, false, rti);
    }
    
    void
    ASTparen::evaluate(Modification& m, int* p, double* width,
                       bool justCheck, int& seedIndex, bool shapeDest,
                       RendererAST* rti) const
    {
        if (mType != ModType) {
            CfdgError::Error(where, "Expression does not evaluate to an adjustment");
            return;
        }
        
        e->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
    }
    
    void
    ASTmodTerm::evaluate(Modification& m, int* p, double* width, 
                        bool justCheck, int& seedIndex, bool shapeDest,
                        RendererAST* rti) const
    {
        double modArgs[6] = {0.0};
        int argcount = 0;
        
        if (args) {
            if (modType != modification && args->mType == NumericType) {
                if (justCheck)
                    argcount = args->evaluate(nullptr, 0);
                else 
                    argcount = args->evaluate(modArgs, 6, rti);
            } else if (modType == modification && args->mType != ModType){
                CfdgError::Error(where, "Adjustments require numeric arguments");
                return;
            }
        }
        
        int minCount = 1;
        int maxCount = 1;
		double arg[6] = {0.0};
        for (int i = 0; i < argcount; ++i)
            arg[i] = fmax(-1.0, fmin(1.0, modArgs[i]));
        
        switch (modType) {
            case ASTmodTerm::x: {
                maxCount = 2;
                if (justCheck) break;
                if (argcount == 1) 
                    modArgs[1] = 0.0;
                agg::trans_affine_translation trx(modArgs[0], modArgs[1]);
                m.m_transform.premultiply(trx);
                break;
            }
            case ASTmodTerm::y: {
                if (justCheck) break;
                agg::trans_affine_translation tr(0.0, modArgs[0]);
                m.m_transform.premultiply(tr);
                break;
            }
            case ASTmodTerm::z: {
                if (justCheck) break;
                agg::trans_affine_1D_translation tr(modArgs[0]);
                m.m_Z.premultiply(tr);
                break;
            }
            case ASTmodTerm::xyz: {
                minCount = maxCount = 3;
                if (justCheck) break;
                agg::trans_affine_translation trx(modArgs[0], modArgs[1]);
                m.m_transform.premultiply(trx);
                agg::trans_affine_1D_translation trz(modArgs[2]);
                m.m_Z.premultiply(trz);
                break;
            }
            case ASTmodTerm::time: {
                minCount = maxCount = 2;
                if (justCheck) break;
                agg::trans_affine_time_translation tr(modArgs[0], modArgs[1]);
                m.m_time.premultiply(tr);
                break;
            }
            case ASTmodTerm::timescale: {
                if (justCheck) break;
                agg::trans_affine_time_scaling sc(modArgs[0]);
                m.m_time.premultiply(sc);
                break;
            }
            case ASTmodTerm::transform: {
                maxCount = 6;
                if (argcount != 1 && argcount != 2 && argcount != 4 && argcount != 6)
                    CfdgError::Error(where, "transform adjustment takes 1, 2, 4, or 6 parameters");
                if (justCheck) break;
                switch (argcount) {
                    case 2:
                    case 1: {
                        if (argcount == 1) 
                            modArgs[1] = 0.0;
                        agg::trans_affine_translation trx(modArgs[0], modArgs[1]);
                        m.m_transform.premultiply(trx);
                        break;
                    }
                    case 4: {
                        agg::trans_affine sq;
                        double dx = modArgs[2] - modArgs[0];
                        double dy = modArgs[3] - modArgs[1];
                        sq.scale(sqrt(dx * dx + dy * dy));
                        sq.rotate(atan2(dy, dx));
                        sq.translate(modArgs[0], modArgs[1]);
                        m.m_transform.premultiply(sq);
                        break;
                    }
                    case 6: {
                        agg::trans_affine par;
                        par.rect_to_parl(0.0, 0.0, 1.0, 1.0, modArgs);
                        m.m_transform.premultiply(par);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case ASTmodTerm::size: {
                maxCount = 2;
                if (justCheck) break;
                if (argcount == 1) 
                    modArgs[1] = modArgs[0];
                agg::trans_affine_scaling sc(modArgs[0], modArgs[1]);
                m.m_transform.premultiply(sc);
                break;
            }
            case ASTmodTerm::sizexyz: {
                minCount = maxCount = 3;
                if (justCheck) break;
                agg::trans_affine_scaling sc(modArgs[0], modArgs[1]);
                m.m_transform.premultiply(sc);
                agg::trans_affine_1D_scaling scz(modArgs[2]);
                m.m_Z.premultiply(scz);
                break;
            }
            case ASTmodTerm::zsize: {
                if (justCheck) break;
                agg::trans_affine_1D_scaling sc(modArgs[0]);
                m.m_Z.premultiply(sc);
                break;
            }
            case ASTmodTerm::rot: {
                if (justCheck) break;
                agg::trans_affine_rotation rot(modArgs[0] * MY_PI / 180.0);
                m.m_transform.premultiply(rot);
                break;
            }
            case ASTmodTerm::skew: {
                minCount = maxCount = 2;
                if (justCheck) break;
                agg::trans_affine_skewing sk(modArgs[0] * MY_PI / 180.0, 
                                             modArgs[1] * MY_PI / 180.0);
                m.m_transform.premultiply(sk);
                break;
            }
            case ASTmodTerm::flip: {
                if (justCheck) break;
                agg::trans_affine_reflection ref(modArgs[0] * MY_PI / 180.0);
                m.m_transform.premultiply(ref);
                break;
            }
            case ASTmodTerm::Entropy: {
                //ent.append(mod->parameter);
                minCount = maxCount = 0;
                //if (justCheck) break;
                m.mRand64Seed.xorString(entString.c_str(), seedIndex);
                break;
            }
            case ASTmodTerm::hue: {
                maxCount = 2;
                if (justCheck) break;
                if (argcount == 1) {
                    if (m.m_ColorAssignment & HSBColor::HueMask) {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest)
                        m.m_Color.h = HSBColor::adjustHue(m.m_Color.h, modArgs[0]);
                    else
                        m.m_Color.h += modArgs[0];
                } else {
                    if ((m.m_ColorAssignment & HSBColor::HueMask ||
                         m.m_Color.h != 0.0))
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest) {
                        m.m_Color.h = HSBColor::adjustHue(m.m_Color.h, arg[0],
                                                          HSBColor::HueTarget,
                                                          modArgs[1]);
                    } else {
                        m.m_Color.h = arg[0];
                        m.m_ColorTarget.h = modArgs[1];
                        m.m_ColorAssignment |= HSBColor::Hue2Value;
                    }
                }
                break;
            }
            case ASTmodTerm::sat: {
                maxCount = 2;
                if (justCheck) break;
                if (argcount == 1) {
                    if ((m.m_ColorAssignment & HSBColor::SaturationMask) ||
                         m.m_Color.s != 0.0)
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest)
                        m.m_Color.s = HSBColor::adjust(m.m_Color.s, arg[0]);
                    else
                        m.m_Color.s = arg[0];
                } else {
                    if ((m.m_ColorAssignment & HSBColor::SaturationMask) ||
                         m.m_Color.s != 0.0 || m.m_ColorTarget.s != 0.0)
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest) {
                        m.m_Color.s = HSBColor::adjust(m.m_Color.s, arg[0], 1, arg[1]);
                    } else {
                        m.m_Color.s = arg[0];
                        m.m_ColorTarget.s = arg[1];
                        m.m_ColorAssignment |= HSBColor::Saturation2Value;
                    }
                }
                break;
            }
            case ASTmodTerm::bright: {
                maxCount = 2;
                if (justCheck) break;
                if (argcount == 1) {
                    if ((m.m_ColorAssignment & HSBColor::BrightnessMask ||
                         m.m_Color.b != 0.0))
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest)
                        m.m_Color.b = HSBColor::adjust(m.m_Color.b, arg[0]);
                    else
                        m.m_Color.b = arg[0];
                } else {
                    if ((m.m_ColorAssignment & HSBColor::BrightnessMask) ||
                         m.m_Color.b != 0.0 || m.m_ColorTarget.b != 0.0)
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest) {
                        m.m_Color.b = HSBColor::adjust(m.m_Color.b, arg[0], 1, arg[1]);
                    } else {
                        m.m_Color.b = arg[0];
                        m.m_ColorTarget.b = arg[1];
                        m.m_ColorAssignment |= HSBColor::Brightness2Value;
                    }
                }
                break;
            }
            case ASTmodTerm::alpha: {
                maxCount = 2;
                if (p)
                    *p |= CF_USES_ALPHA;
                if (justCheck) break;
                if (argcount == 1) {
                    if ((m.m_ColorAssignment & HSBColor::AlphaMask ||
                         m.m_Color.a != 0.0))
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest)
                        m.m_Color.a = HSBColor::adjust(m.m_Color.a, arg[0]);
                    else
                        m.m_Color.a = arg[0];
                } else {
                    if ((m.m_ColorAssignment & HSBColor::AlphaMask) ||
                         m.m_Color.a != 0.0 || m.m_ColorTarget.a != 0.0)
                    {
                        if (rti == nullptr)
                            throw DeferUntilRuntime();
                        if (!shapeDest)
                            RendererAST::ColorConflict(rti, where);
                    }
                    if (shapeDest) {
                        m.m_Color.a = HSBColor::adjust(m.m_Color.a, arg[0], 1, arg[1]);
                    } else {
                        m.m_Color.a = arg[0];
                        m.m_ColorTarget.a = arg[1];
                        m.m_ColorAssignment |= HSBColor::Alpha2Value;
                    }
                }
                break;
            }
            case ASTmodTerm::hueTarg: {
                if (justCheck) break;
                if ((m.m_ColorAssignment & HSBColor::HueMask) ||
                     m.m_Color.h != 0.0)
                {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest) {
                    m.m_Color.h = HSBColor::adjustHue(m.m_Color.h, arg[0],
                                                      HSBColor::HueTarget,
                                                      m.m_ColorTarget.h);
                } else {
                    m.m_Color.h = arg[0];
                    m.m_ColorAssignment |= HSBColor::HueTarget;
                }
                break;
            }
            case ASTmodTerm::satTarg: {
                if (justCheck) break;
                if ((m.m_ColorAssignment & HSBColor::SaturationMask) ||
                     m.m_Color.s != 0.0)
                {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest) {
                    m.m_Color.s = HSBColor::adjust(m.m_Color.s, arg[0], 1,
                                                   m.m_ColorTarget.s);
                } else {
                    m.m_Color.s = arg[0];
                    m.m_ColorAssignment |= HSBColor::SaturationTarget;
                }
                break;
            }
            case ASTmodTerm::brightTarg: {
                if (justCheck) break;
                if ((m.m_ColorAssignment & HSBColor::BrightnessMask) ||
                     m.m_Color.b != 0.0)
                {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest) {
                    m.m_Color.b = HSBColor::adjust(m.m_Color.b, arg[0], 1,
                                                   m.m_ColorTarget.b);
                } else {
                    m.m_Color.b = arg[0];
                    m.m_ColorAssignment |= HSBColor::BrightnessTarget;
                }
                break;
            }
            case ASTmodTerm::alphaTarg: {
                if (p)
                    *p |= CF_USES_ALPHA;
                if (justCheck) break;
                if ((m.m_ColorAssignment & HSBColor::AlphaMask) ||
                     m.m_Color.a != 0.0)
                {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest) {
                    m.m_Color.a = HSBColor::adjust(m.m_Color.a, arg[0], 1,
                                                   m.m_ColorTarget.a);
                } else {
                    m.m_Color.a = arg[0];
                    m.m_ColorAssignment |= HSBColor::AlphaTarget;
                }
                break;
            }
            case ASTmodTerm::targHue: {
                if (justCheck) break;
                m.m_ColorTarget.h += modArgs[0];
                break;
            }
            case ASTmodTerm::targSat: {
                if (justCheck) break;
                if (m.m_ColorTarget.s != 0.0) {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest)
                    m.m_ColorTarget.s = HSBColor::adjust(m.m_ColorTarget.s, arg[0]);
                else
                    m.m_ColorTarget.s = arg[0];
                break;
            }
            case ASTmodTerm::targBright: {
                if (justCheck) break;
                if (m.m_ColorTarget.b != 0.0) {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest)
                    m.m_ColorTarget.b = HSBColor::adjust(m.m_ColorTarget.b, arg[0]);
                else
                    m.m_ColorTarget.b = arg[0];
                break;
            }
            case ASTmodTerm::targAlpha: {
                if (justCheck) break;
                if (m.m_ColorTarget.a != 0.0) {
                    if (rti == nullptr)
                        throw DeferUntilRuntime();
                    if (!shapeDest)
                        RendererAST::ColorConflict(rti, where);
                }
                if (shapeDest)
                    m.m_ColorTarget.a = HSBColor::adjust(m.m_ColorTarget.a, arg[0]);
                else
                    m.m_ColorTarget.a = arg[0];
                break;
            }
            case ASTmodTerm::param: {
                minCount = maxCount = 0;
                if (!p) {
                    CfdgError::Error(where, "Cannot provide a parameter in this context");
                    break;
                }
                if (justCheck) break;
                if (!entString.empty()) {
                    if (entString.find("evenodd") != std::string::npos)
                        *p |= CF_EVEN_ODD;
                    if (entString.find("iso") != std::string::npos)
                        *p |= CF_ISO_WIDTH;
                    if (entString.find("join") != std::string::npos)
                        *p &= ~CF_JOIN_MASK;
                    if (entString.find("miterjoin") != std::string::npos)
                        *p |= CF_MITER_JOIN | CF_JOIN_PRESENT;
                    if (entString.find("roundjoin") != std::string::npos)
                        *p |= CF_ROUND_JOIN | CF_JOIN_PRESENT;
                    if (entString.find("beveljoin") != std::string::npos)
                        *p |= CF_BEVEL_JOIN | CF_JOIN_PRESENT;
                    if (entString.find("cap") != std::string::npos)
                        *p &= ~CF_CAP_MASK;
                    if (entString.find("buttcap") != std::string::npos)
                        *p |= CF_BUTT_CAP | CF_CAP_PRESENT;
                    if (entString.find("squarecap") != std::string::npos)
                        *p |= CF_SQUARE_CAP | CF_CAP_PRESENT;
                    if (entString.find("roundcap") != std::string::npos)
                        *p |= CF_ROUND_CAP | CF_CAP_PRESENT;
                }
                break;
            }
            case ASTmodTerm::stroke: {
                if (!width) {
                    CfdgError::Error(where, "Cannot provide a stroke width in this context");
                    break;
                }
                if (justCheck) break;
                *width = modArgs[0];
                break;
            }
            case ASTmodTerm::modification: {
                minCount = maxCount = 0;
                if (rti == nullptr) {
                    const ASTmodification* mod = dynamic_cast<const ASTmodification*>(args.get());
                    if (!mod || (mod->modClass & (ASTmodification::HueClass |
                                                  ASTmodification::HueTargetClass |
                                                  ASTmodification::BrightClass |
                                                  ASTmodification::BrightTargetClass |
                                                  ASTmodification::SatClass |
                                                  ASTmodification::SatTargetClass |
                                                  ASTmodification::AlphaClass |
                                                  ASTmodification::AlphaTargetClass)))
                    {
                        throw DeferUntilRuntime();
                    }
                }
                args->evaluate(m, p, width, justCheck, seedIndex, shapeDest, rti);
                break;
            }
            default:
                break;
        }
        
        if (argcount < minCount)
            CfdgError::Error(where, "Not enough adjustment parameters");
        if (argcount > maxCount)
            CfdgError::Error(where, "Too many adjustment parameters");
    }
    
    void
    ASTfunction::entropy(std::string& ent) const
    {
        if (functype <= NotAFunction) return;
        
        arguments->entropy(ent);
        ent.append(Entropies[functype]);
    }
    
    void
    ASTselect::entropy(std::string& e) const
    {
        e.append(ent);
    }
    
    void
    ASTruleSpecifier::entropy(std::string& ent) const
    {
        ent.append(entropyVal);
    }
    
    void
    ASTcons::entropy(std::string& ent) const
    {
        for (size_t i = 0; i < children.size(); ++i)
            children[i]->entropy(ent);
        ent.append("\xC5\x60\xA5\xC5\xC8\x74");
    }
    
    void
    ASTreal::entropy(std::string& ent) const
    {
        ent.append(text);
    }
    
    void
    ASTvariable::entropy(std::string& ent) const
    {
        ent.append(text);
    }
    
    void
    ASTuserFunction::entropy(std::string& ent) const
    {
        if (arguments)
            arguments->entropy(ent);
        ent.append(definition->mName);
    }
    
    void
    ASToperator::entropy(std::string& ent) const
    {
        left->entropy(ent);
        if (right) right->entropy(ent);
        
        // These random strings are courtesy of http://www.fourmilab.ch/hotbits/
        switch (op) {
            case '*':
                ent.append("\x2E\x32\xD9\x2C\x41\xFE");
                break;
            case '/':
                ent.append("\x6B\x15\x23\x41\x9E\xEB");
                break;
            case '+':
                ent.append("\xD7\xB1\xB0\x39\x33\xC8");
                break;
            case '-':
                ent.append("\x5D\xE7\xF0\x94\xC4\x13");
                break;
            case '^':
                ent.append("\x02\x3C\x68\x36\xC5\xA0");
                break;
            case 'N':
                ent.append("\x55\x89\x51\x46\xDB\x84");
                break;
            case 'P':
                ent.append("\x8E\xAC\x29\x4B\x0E\xDC");
                break;
            case '!':
                ent.append("\x19\x3A\x3E\x53\x14\xEA");
                break;
            case '<':
                ent.append("\xBE\xDB\xC4\xA6\x4E\xAD");
                break;
            case '>':
                ent.append("\xC7\xD9\x57\x32\xD6\x87");
                break;
            case 'L':
                ent.append("\xE3\x56\x7E\x44\x57\x80");
                break;
            case 'G':
                ent.append("\xB1\x2D\x2A\xCC\x2C\x40");
                break;
            case '=':
                ent.append("\x78\x48\xC2\x95\xA9\xE2");
                break;
            case 'n':
                ent.append("\x36\xCC\x01\x3B\x2F\xAD");
                break;
            case '&':
                ent.append("\x28\x9B\xFB\x7F\xDB\x9C");
                break;
            case '|':
                ent.append("\x2E\x40\x1B\x44\x15\x7C");
                break;
            case 'X':
                ent.append("\xA7\x2B\x92\xFA\xFC\xF9");
                break;
            default:
                ent.append("\x60\x2F\x10\xAD\x10\xFF");
                break;
        }
    }
    
    void
    ASTparen::entropy(std::string& ent) const
    {
        e->entropy(ent);
        ent.append("\xE8\xE9\xF6\x7E\x1A\xF1");
    }
    
    void
    ASTmodTerm::entropy(std::string& ent) const
    {
        if (args) args->entropy(ent);
        ent.append(ASTmodTerm::Entropies[modType]);
    }
    
    void
    ASTarray::entropy(std::string& e) const
    {
        e.append(entString);
    }
    
    ASTexpression*
    ASTfunction::simplify()
    {
        if (isConstant) {
            double result;
            if (evaluate(&result, 1) != 1) {
                return this;
            }
            
            ASTreal* r = new ASTreal(result, where);
            r->isNatural = isNatural;
            r->isLocal = isLocal;
            
            delete this;
            return r;
        } else {
            arguments.reset(arguments.release()->simplify());
        }
        
        return this;
    }
    
    ASTexpression*
    ASTselect::simplify()
    {
        if (!indexCache) {
            arguments.reset(arguments.release()->simplify());
            return this;
        }
        
        ASTexpression* chosenOne = (*arguments)[indexCache];
        if (!arguments->release(indexCache))
            return this;
        
        delete this;
        return chosenOne->simplify();
    }
    
    ASTexpression*
    ASTruleSpecifier::simplify()
    {
        if (arguments) {
            if (ASTcons* carg = dynamic_cast<ASTcons*>(arguments.get())) {
                for (size_t i = 0; i < carg->children.size(); ++i)
                    carg->children[i].reset(carg->children[i].release()->simplify());
            } else {
                arguments.reset(arguments.release()->simplify());
            }
        }
        return this;
    }
    
    ASTexpression*
    ASTcons::simplify()
    {
        if (children.size() == 1) {
            ASTexpression* ret = children[0].release()->simplify();
            delete this;
            return ret;
        }
        for (size_t i = 0; i < children.size(); ++i)
            children[i].reset(children[i].release()->simplify());
        return this;
    }
    
    ASTexpression*
    ASTuserFunction::simplify()
    {
        if (arguments) {
            if (ASTcons* carg = dynamic_cast<ASTcons*>(arguments.get())) {
                // Can't use ASTcons::simplify() because it will collapse the
                // ASTcons if it only has one child and that will break the
                // function arguments.
                for (size_t i = 0; i < carg->children.size(); ++i)
                    carg->children[i].reset(carg->children[i].release()->simplify());
            } else {
                arguments.reset(arguments.release()->simplify());
            }
        }
        return this;
    }
    
    ASTexpression*
    ASToperator::simplify()
    {
        left.reset(left.release()->simplify());
        if (right) right.reset(right.release()->simplify());
        
        if (isConstant && (mType == NumericType || mType == FlagType)) {
            double result;
            if (evaluate(&result, 1) != 1) {
                return nullptr;
            }
            
            ASTreal* r = new ASTreal(result, where);
            r->mType = mType;
            r->isNatural = isNatural;
            r->isLocal = isLocal;
            
            delete this;
            return r;
        }
        
        return this;
    }
    
    ASTexpression*
    ASTparen::simplify()
    {
        ASTexpression* e2 = e.release()->simplify();
        
        delete this;
        return e2;
    }
    
    ASTvariable::ASTvariable(int stringNum, const std::string& str, const yy::location& loc) 
    : ASTexpression(loc), stringIndex(stringNum), text(str), stackIndex(0),
      isParameter(false) { };
    
    ASTexpression*
    ASTmodTerm::simplify()
    {
        if (args) {
            args.reset(args.release()->simplify());
        }
        return this;
    }
    
    ASTexpression*
    ASTarray::simplify()
    {
        if (!isConstant) {
            mArgs.reset(mArgs.release()->simplify());
            return this;
        }
        
        double i;
        if (mArgs->evaluate(&i, 1) != 1) {
            CfdgError::Error(mArgs->where, "Cannot evaluate array index");
            return this;
        }
        int index = static_cast<int>(i);
        if ((mLength - 1) * mStride + index >= mCount || index < 0) {
            CfdgError::Error(where, "Array index exceeds bounds");
            return this;
        }
        
        // Create a new cons-list based on the evaluated variable's expression
        ASTreal* top = new ASTreal(mData[index], where);
        top->text = entString;                // use variable name for entropy
        ASTexpression* list = top;
        for (int i = 1; i < mLength; ++i)
            list = list->append(new ASTreal(mData[i * mStride + index], where));
        list->isNatural = isNatural;
        delete this;
        return list;
    }

    void
    ASTmodification::evalConst() 
    {
        int nonConstant = 0;
        
        ASTtermArray temp;
        temp.swap(modExp);

        for (term_ptr& mod: temp) {
            bool keepThisOne = false;
            if (!mod) {
                CfdgError::Error(where, "Unknown term in shape adjustment");
                continue;
            }
            
            // Put in code for separating color changes and target color changes
                        
            int mc = ASTmodification::ModClass[mod->modType];
            modClass |= mc;
            if (!mod->isConstant)
                nonConstant |= mc;
            bool justCheck = (mc & nonConstant) != 0;
            
            try {
                mod->evaluate(modData, &flags, &strokeWidth, justCheck, entropyIndex, false, nullptr);
            } catch (DeferUntilRuntime&) {
                keepThisOne = true;
            }
            
            if (justCheck || keepThisOne) {
                if (mod->args)
                    mod->args.reset(mod->args.release()->simplify());
                modExp.push_back(std::move(mod));
            }
        }
    }
    
    void
    ASTmodification::addEntropy(const std::string& s)
    {
        modData.mRand64Seed.xorString(s.c_str(), entropyIndex);
    }
    
    unsigned
    ASTselect::getIndex(RendererAST* rti) const
    {
        if (indexCache)
            return indexCache;

        double select = 0.0;
        (*arguments)[0]->evaluate(&select, 1, rti);

        if (ifSelect)
            return select ? 1 : 2;

        int i = static_cast<int>(select) + 1;

        if (i <= 0)
            return 1;
        if (i > arguments->size())
            return arguments->size() - 1;
        return i;
    }
}

