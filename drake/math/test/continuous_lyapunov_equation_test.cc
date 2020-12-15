#include "drake/math/continuous_lyapunov_equation.h"

#include <limits>
#include <stdexcept>
#include <vector>

#include <Eigen/Core>
#include <gtest/gtest.h>

#include "drake/common/test_utilities/eigen_matrix_compare.h"

namespace drake {
namespace math {
namespace {

using Eigen::MatrixXd;

const double kTolerance = 5 * std::numeric_limits<double>::epsilon();

void SolveRealLyapunovEquationAndVerify(const Eigen::Ref<const MatrixXd>& A,
                                        const Eigen::Ref<const MatrixXd>& Q) {
  MatrixXd X{RealContinuousLyapunovEquation(A, Q)};
  // Check that X is symmetric
  EXPECT_TRUE(CompareMatrices(X, X.transpose(), 5 * kTolerance,
                              MatrixCompareType::absolute));
  // Check that X is the solution to the continuous Lyapunov equation.
  EXPECT_TRUE(CompareMatrices(A.transpose() * X + X * A, -Q,
                              5 * kTolerance * Q.norm(),
                              MatrixCompareType::absolute));
}

GTEST_TEST(RealContinuousLyapunovEquation, ThrowInvalidSizedMatricesTest) {
  // This tests if the exceptions are thrown for invalidly sized matrices. A and
  // Q need to be square and of same size.
  const int n{1}, m{2};

  // non-square A
  MatrixXd A1(n, m);
  A1 << 1, 1;
  MatrixXd Q1(m, m);
  Q1 << 1, 1, 1, 1;

  // non-square Q
  MatrixXd A2(m, m);
  A2 << 1, 1, 1, 1;
  MatrixXd Q2(n, m);
  Q2 << 1, 1;

  // not-same sized
  MatrixXd A3(m, m);
  A3 << 1, 1, 1, 1;
  MatrixXd Q3(n, n);
  Q3 << 1;

  std::vector<MatrixXd> As{A1, A2, A3};
  std::vector<MatrixXd> Qs{Q1, Q2, Q3};

  for (int i = 0; i < static_cast<int>(As.size()); ++i) {
    EXPECT_ANY_THROW(RealContinuousLyapunovEquation(As[i], Qs[i]));
  }
}

GTEST_TEST(RealContinuousLyapunovEquation, ThrowEigenValuesATest) {
  // Given the Eigenvalues of @param A as lambda_1, ..., lambda_n, then the
  // solution is unique if and only if lambda_i + lambda_j != 0 for all  i, j.
  // (see https://www.mathworks.com/help/control/ref/dlyap.html)
  // This tests if an exception is thrown if the eigenvalues violate this
  // requirement.
  int n{2};
  // complex pair of eigenvalues that adds to zero
  MatrixXd A1(n, n);
  A1 << 0, 1, -1, 0;
  // 0 eigenvalue
  MatrixXd A2(n, n);
  A2 << 0, 0, 0, -1;
  // eigenvalue within tol of zero
  MatrixXd A3(n, n);
  A3 << 1, 0, 0, -1e-11;
  // sum of eigenvalues within tol of zero
  MatrixXd A4(n, n);
  A4 << -1 + 1e-10, 0, 0, 1 - 5e-11;
  MatrixXd Q(n, n);
  Q << 1, 0, 0, 1;

  EXPECT_ANY_THROW(RealContinuousLyapunovEquation(A1, Q));
  EXPECT_ANY_THROW(RealContinuousLyapunovEquation(A2, Q));
  EXPECT_ANY_THROW(RealContinuousLyapunovEquation(A3, Q));
  EXPECT_ANY_THROW(RealContinuousLyapunovEquation(A4, Q));
}

GTEST_TEST(RealContinuousLyapunovEquation, Solve1by1Test) {
  // This is a simple 1-by-1 test case, it tests the internal 1-by-1 solver.
  const int n{1};
  MatrixXd A(n, n);
  A << -1;
  MatrixXd Q(n, n);
  Q << 1;
  MatrixXd X(n, n);
  X << 0.5;
  EXPECT_TRUE(CompareMatrices(
      internal::Solve1By1RealContinuousLyapunovEquation(A.transpose(), Q), X,
      kTolerance, MatrixCompareType::absolute));
  SolveRealLyapunovEquationAndVerify(A, Q);
}

GTEST_TEST(RealContinuousLyapunovEquation, Solve2by2Test) {
  // Example1 from https://www.mathworks.com/help/control/ref/lyap.html
  // Note that Matlab solves AX + XA' + Q = 0
  // Furthermore it tests the internal 2-by-2 solver.
  const int n{2};
  MatrixXd A(n, n);
  A << 1, 2, -3, -4;
  MatrixXd X(n, n);
  X << 6.0 + 1.0 / 6.0, -(3.0 + 5.0 / 6.0), -(3.0 + 5.0 / 6.0), 3;

  MatrixXd Q_internal(n, n);
  Q_internal << 3, 1, NAN, 1;
  EXPECT_TRUE(CompareMatrices(internal::Solve2By2RealContinuousLyapunovEquation(
                                  A.transpose(), Q_internal),
                              X, 4 * kTolerance, MatrixCompareType::absolute));

  MatrixXd Q(n, n);
  Q << 3, 1, 1, 1;
  EXPECT_TRUE(CompareMatrices(RealContinuousLyapunovEquation(A.transpose(), Q),
                              X, 4 * kTolerance, MatrixCompareType::absolute));
  SolveRealLyapunovEquationAndVerify(A.transpose(), Q);
}

GTEST_TEST(RealContinuousLyapunovEquation, Solve3by3Test1) {
  // Tests if a 3-by-3 problem is reduced.
  const int n{3};
  MatrixXd A(n, n);
  A << -MatrixXd::Identity(n, n);
  MatrixXd Q(n, n);
  Q << MatrixXd::Identity(n, n);

  SolveRealLyapunovEquationAndVerify(A, Q);
}

GTEST_TEST(RealContinuousLyapunovEquation, Solve3by3Test2) {
  // The system has eigenvalues: lambda_1/2  = -0.5000 +/- 0.8660i
  // and lambda_3 = -1. Therefore, there exists a 2-by-2 block on the
  // diagonal.
  // The compared solution is generated by matlab's lyap function.

  const int n{3};
  MatrixXd A(n, n);
  A << 0, 1, 0, -1, -1, 0, 0, 0, -1;
  MatrixXd Q(n, n);
  Q << MatrixXd::Identity(n, n);
  MatrixXd X(n, n);
  X << 1.5, 0.5, 0, 0.5, 1, 0, 0, 0, 0.5;

  EXPECT_TRUE(CompareMatrices(RealContinuousLyapunovEquation(A, Q), X,
                              4 * kTolerance, MatrixCompareType::absolute));
  SolveRealLyapunovEquationAndVerify(A, Q);
}

GTEST_TEST(RealContinuousLyapunovEquation, Solve4by4Test1) {
  // The system has eigenvalues: lambda_1/2  = -0.5000 +/- 0.8660i
  // and lambda_3/4 = -1. Therefore, there exists a 2-by-2 block on the
  // diagonal.

  const int n{4};
  MatrixXd A(n, n);
  A << -1, 0, 0, 0, 0, 0, 1, 0, 0, -1, -1, 0, 0, 0, 0, -1;
  MatrixXd Q(n, n);
  Q << MatrixXd::Identity(n, n);
  SolveRealLyapunovEquationAndVerify(A, Q);

  MatrixXd A2(n, n);
  A2 << -1, 0.43, -1.5, 0.2, 0, 0, 1, 0, 0, -1, -1, 0, 0, 0, 0, -1;
  SolveRealLyapunovEquationAndVerify(A2, Q);
}

GTEST_TEST(RealContinuousLyapunovEquation, Solve10by10) {
  // We test the code on a large example. We generate a random matrix A_half
  // with Matlab's rand(10).
  // The Matrix A = -A_half*A_half' has eigenvalues =< 0 by construction.

  MatrixXd A_half(10, 10);
  A_half << 0.1622, 0.4505, 0.1067, 0.4314, 0.8530, 0.4173, 0.7803, 0.2348,
      0.5470, 0.9294, 0.7943, 0.0838, 0.9619, 0.9106, 0.6221, 0.0497, 0.3897,
      0.3532, 0.2963, 0.7757, 0.3112, 0.2290, 0.0046, 0.1818, 0.3510, 0.9027,
      0.2417, 0.8212, 0.7447, 0.4868, 0.5285, 0.9133, 0.7749, 0.2638, 0.5132,
      0.9448, 0.4039, 0.0154, 0.1890, 0.4359, 0.1656, 0.1524, 0.8173, 0.1455,
      0.4018, 0.4909, 0.0965, 0.0430, 0.6868, 0.4468, 0.6020, 0.8258, 0.8687,
      0.1361, 0.0760, 0.4893, 0.1320, 0.1690, 0.1835, 0.3063, 0.2630, 0.5383,
      0.0844, 0.8693, 0.2399, 0.3377, 0.9421, 0.6491, 0.3685, 0.5085, 0.6541,
      0.9961, 0.3998, 0.5797, 0.1233, 0.9001, 0.9561, 0.7317, 0.6256, 0.5108,
      0.6892, 0.0782, 0.2599, 0.5499, 0.1839, 0.3692, 0.5752, 0.6477, 0.7802,
      0.8176, 0.7482, 0.4427, 0.8001, 0.1450, 0.2400, 0.1112, 0.0598, 0.4509,
      0.0811, 0.7948;
  MatrixXd A{-A_half * A_half.transpose()};
  MatrixXd Q{MatrixXd::Identity(10, 10)};

  // The solution X is obtained by Matlab's lyap(A.',Q)
  MatrixXd X(10, 10);
  X << 5.174254345982084, 3.785962224550206, 1.716851637434820,
      -6.423467487688685, -3.303527757978912, 7.751563477958063,
      -5.453159309169113, 2.756394136066010, -2.383245959863380,
      -4.646704649671120, 3.785962224550206, 7.733223722073816,
      0.984667079496413, -6.985751984700270, -1.468117803443308,
      -2.381962895250860, -11.406359384231266, 13.403654956780908,
      -7.905663634873605, -1.707241841788795, 1.716851637434820,
      0.984667079496413, 2.810911691014975, -2.143076146699036,
      -2.568865412823195, 7.579636343964955, 0.989231265555543,
      -4.122828484247153, 0.221166408736615, -3.501510532379084,
      -6.423467487688685, -6.985751984700270, -2.143076146699036,
      11.153852606907163, 2.424134196572830, -6.287532769413548,
      9.904445394226688, -9.890648864864904, 7.335273514428504,
      4.356558308557354, -3.303527757978912, -1.468117803443308,
      -2.568865412823195, 2.424134196572830, 5.366429856975694,
      -11.563947250836353, 0.393445687076630, 5.444872146647519,
      -2.596780779003215, 6.133050237127323, 7.751563477958063,
      -2.381962895250860, 7.579636343964955, -6.287532769413548,
      -11.563947250836353, 42.514033344951628, 11.168249111715349,
      -29.261574349736009, 12.223632134534295, -18.633242175973727,
      -5.453159309169113, -11.406359384231266, 0.989231265555543,
      9.904445394226688, 0.393445687076630, 11.168249111715349,
      21.520015757259888, -27.074863900080999, 12.930264173939383,
      -0.821271729309166, 2.756394136066010, 13.403654956780908,
      -4.122828484247153, -9.890648864864904, 5.444872146647519,
      -29.261574349736009, -27.074863900080999, 42.402987995831381,
      -20.932210488385589, 9.041568418134542, -2.383245959863380,
      -7.905663634873605, 0.221166408736615, 7.335273514428504,
      -2.596780779003215, 12.223632134534295, 12.930264173939383,
      -20.932210488385589, 13.535693361419060, -4.079542688309729,
      -4.646704649671120, -1.707241841788795, -3.501510532379084,
      4.356558308557354, 6.133050237127323, -18.633242175973727,
      -0.821271729309166, 9.041568418134542, -4.079542688309729,
      10.282049375996213;

  EXPECT_TRUE(CompareMatrices(RealContinuousLyapunovEquation(A, Q), X, 1E-10,
                              MatrixCompareType::absolute));
}

}  // namespace
}  // namespace math
}  // namespace drake
