/* 

   This file is part of the regenie software package.

   Copyright (c) 2020-2024 Joelle Mbatchou, Andrey Ziyatdinov & Jonathan Marchini

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.

*/

#ifndef MODELS_H
#define MODELS_H

#define ETAMINTHR -30.0
#define ETAMAXTHR 30.0

struct ests {

  Eigen::MatrixXd offset_nullreg;
  Eigen::MatrixXd blups, ltco_prs;
  Eigen::MatrixXd Gamma_sqrt, Gamma_sqrt_mask, Y_hat_p;
  std::vector<Eigen::MatrixXd> X_Gamma;
  Eigen::MatrixXd bhat_start; // for interaction tests
  std::vector<cox_mle> cox_MLE_NULL;
  std::vector<survival_data> survival_data_pheno;

};

struct ridgel0 {

  Eigen::MatrixXd GGt;
  Eigen::MatrixXd GTY;
  std::vector<Eigen::MatrixXd> G_folds, GtY; // storage for folds at levle 0
  Eigen::MatrixXd GGt_eig_vec, GGt_eig_val;
  Eigen::MatrixXd Wmat, ymat_res;
  MatrixXb picked_top_snp;
  bool subset_l0_snps_gmat = false;
  Eigen::ArrayXi nspns_picked_block, nspns_picked, indices_gmat_keep;
};

struct ridgel1 {

  std::vector<Eigen::MatrixXd> X_folds, XtY; // storage for folds at level 1
  std::vector<std::vector<Eigen::MatrixXd>> pred_mat, test_mat;
  std::vector<Eigen::MatrixXd> test_mat_conc;
  std::vector<std::vector<Eigen::MatrixXd>> pred_pheno, test_pheno;
  std::vector<std::vector<Eigen::MatrixXd>> pred_pheno_raw, test_pheno_raw;
  std::vector<std::vector<Eigen::MatrixXd>> pred_offset, test_offset;
  std::vector<Eigen::VectorXi> fold_id; //cox cv fold id
  std::vector<Eigen::MatrixXd> cumsum_values, cumsum_values_full; // storage of values to compute rsq and values [Sx, Sy, Sx2, Sy2, Sxy]
  std::vector<std::vector<Eigen::MatrixXd>> beta_hat_level_1;
  ArrayXb pheno_l1_not_converged;
  MatrixXb l0_colkeep;
  Eigen::MatrixXd l0_pv_block;
  Eigen::ArrayXi chrom_block, chrom_map_ndiff;
  Eigen::ArrayXd ridge_param_mult;
  Eigen::MatrixXd beta_snp_step1; // MxR
  std::vector<Eigen::MatrixXd> top_snp_pgs;
};


void fit_null_logistic(bool const&,const int&,struct param*,struct phenodt*,struct ests*,struct in_files*,mstream&,bool const& save_betas = false);
bool fit_logistic(const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::MatrixXd>&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const ArrayXb>&,Eigen::ArrayXd&,Eigen::ArrayXd&,Eigen::ArrayXd&,struct param const*,mstream&,bool const&,double const& numtol = 1e-8);
double get_logist_dev(const Eigen::Ref<const Eigen::ArrayXd>& Y, const Eigen::Ref<const Eigen::ArrayXd>& pi, const Eigen::Ref<const ArrayXb>& mask);

void fit_null_poisson(const int&,struct param*,struct phenodt*,struct ests*,struct in_files*,mstream&,bool const& save_betas = false);
bool fit_poisson(const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::MatrixXd>&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const ArrayXb>&,Eigen::ArrayXd&,Eigen::ArrayXd&,Eigen::ArrayXd&,struct param const*,mstream&);
double get_poisson_dev(const Eigen::Ref<const Eigen::ArrayXd>& Y, const Eigen::Ref<const Eigen::ArrayXd>& pi, const Eigen::Ref<const ArrayXb>& mask);

void fit_null_cox(bool const&, const int&, struct param*, struct phenodt*, struct ests*, struct in_files*, mstream&, bool const& save_betas = false);
double getCoxLambdaMax(const Eigen::MatrixXd&, const Eigen::VectorXd&);

void ridge_level_0(const int&,struct in_files*,struct param*,struct filter*,struct ests*,struct geno_block*,struct phenodt*,std::vector<snp>&,struct ridgel0*,struct ridgel1*,std::vector<MatrixXb>&,mstream&);
void ridge_level_0_loocv(const int,struct in_files*,struct param*,struct filter*,struct ests*,struct geno_block*,struct phenodt*,std::vector<snp>&,struct ridgel0*,struct ridgel1*,mstream&);
void write_l0_file(std::ofstream*,Eigen::MatrixXd&,mstream&);

void set_mem_l1(struct in_files*,struct param*,struct filter*,struct ests*,struct geno_block*,struct phenodt*,struct ridgel1*,std::vector<MatrixXb>&,mstream&);
void ridge_level_1(struct in_files*,struct param*,struct phenodt*,struct ridgel1*,mstream&);
void ridge_level_1_loocv(struct in_files*,struct param*,struct phenodt*,struct ridgel1*,mstream&);

void ridge_logistic_level_1(struct in_files*,struct param*,struct phenodt*,struct ridgel1*,std::vector<MatrixXb>&,mstream&);
void ridge_logistic_level_1_loocv(struct in_files*,struct param*,struct phenodt*,struct ests*,struct ridgel1*,mstream&);
bool run_log_ridge_loocv(const double&,const Eigen::Ref<const Eigen::ArrayXd>&,const int&,const int&,Eigen::ArrayXd&,Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const Eigen::ArrayXd>&,Eigen::Ref<Eigen::MatrixXd>,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const ArrayXb>&,struct param*,mstream&);
void run_log_ridge_loocv_adam(const int&,const double&,const Eigen::Ref<const Eigen::ArrayXd>&,Eigen::ArrayXd&,Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const Eigen::ArrayXd>&,Eigen::Ref<Eigen::MatrixXd>,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const ArrayXb>&,struct param*,mstream&);

void ridge_poisson_level_1(struct in_files*,struct param*,struct phenodt*,struct ridgel1*,std::vector<MatrixXb>&,mstream&);
void ridge_poisson_level_1_loocv(struct in_files*,struct param*,struct phenodt*,struct ests*,struct ridgel1*,mstream&);
bool run_ct_ridge_loocv(const double&,const Eigen::Ref<const Eigen::ArrayXd>&,const int&,const int&,Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const Eigen::ArrayXd>&,Eigen::Ref<Eigen::MatrixXd>,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const ArrayXb>&,struct param*,mstream&);

void ridge_cox_level_1(struct in_files*, struct param*, struct phenodt*, struct ridgel1*, struct ests*, mstream&);

void get_wvec(Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const ArrayXb>&);
bool get_wvec(Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const ArrayXb>&,const double&);
void get_pvec(Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::MatrixXd>&,double const&);
void get_pvec(Eigen::ArrayXd&,Eigen::ArrayXd&,const double&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::VectorXd>&,double const&);
void get_pvec(Eigen::ArrayXd&,const Eigen::Ref<const Eigen::ArrayXd>&,double const&);
void get_pvec_poisson(Eigen::ArrayXd&,Eigen::ArrayXd&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::MatrixXd>&,double const&);
double compute_log_lik_bern(const double&,const double&);
double compute_log_lik_poisson(const double&,const double&);
double y_log_ypi(const double&,const double&);
double get_deviance_logistic(const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const Eigen::ArrayXd>&,const Eigen::Ref<const ArrayXb>&);

void test_assoc_block(int const&,int const&,struct ridgel0&,struct ridgel1&,struct geno_block*,struct phenodt*,snp const*,struct param const&,mstream&);
void apply_iter_cond(int const&,int const&,int const&,struct ridgel0&,struct ridgel1&,struct geno_block*,snp const*,struct param const&);
void read_l0(int const&,int const&,struct in_files*,struct param*,struct ridgel1*,mstream&);
void read_l0_chunk(int const&,int const&,int const&,int const&,const std::string&,struct param*,struct ridgel1*,mstream&);
void check_l0(int const&,int const&,struct param*,struct ridgel1*,struct phenodt const*,mstream&,bool const& silent_mode = false);


uint64 getSize(std::string const& fname);
#endif

