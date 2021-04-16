/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

//! From jet
//! The row of FdmMatrix3 where row corresponds to (i, j, k) grid point.
struct FdmMatrixRow3 {
	//! Diagonal component of the matrix (row, row).
	ftype Center = 0.0;

	//! Off-diagonal element where colum refers to (i+1, j, k) grid point.
	ftype Right = 0.0;

	//! Off-diagonal element where column refers to (i, j+1, k) grid point.
	ftype Up = 0.0;

	//! OFf-diagonal element where column refers to (i, j, k+1) grid point.
	ftype Front = 0.0;
};

//!
//! \brief 3-D finite difference-type linear system solver using incomplete
//!        Cholesky conjugate gradient (ICCG).
//!
class TFdmIccgSolver
{
public:
	TFdmIccgSolver(const vector3di& InSize);
	~TFdmIccgSolver();

	void Solve(const TArray3<FdmMatrixRow3>& A, const TArray3<ftype>& b, TArray3<ftype>& x);
private:
	vector3di Size;
	struct Preconditioner final {
		TArray3<ftype> d;
		TArray3<ftype> y;

		void Build(const TArray3<FdmMatrixRow3>& matrix, const vector3di& Size);

		void Solve(const TArray3<FdmMatrixRow3>& A, const TArray3<ftype>& b, TArray3<ftype>& x);
	};

	TArray3<ftype> _r;
	TArray3<ftype> _d;
	TArray3<ftype> _q;
	TArray3<ftype> _s;
	Preconditioner _precond;
};