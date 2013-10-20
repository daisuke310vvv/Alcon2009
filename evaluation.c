#include "evaluation.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>

#include "alcon2009.h"

#define PENALTY 10000.0
#define EPSILON 1e-15


/*------------------------------------------------------------*/
/* 各正解最小矩形に対応する最小矩形の候補を絞り込む           */
/*------------------------------------------------------------*/
static void create_candidate_list(object *gt, int n_gt, object *c, int n_c, double **cand_list)
{
	double gt_r =  TRUE_NEGATIVE_AREA_RATE;
	double  c_r = FALSE_POSITIVE_AREA_RATE;

	int i, j;

	for (i = 0; i < n_gt; i++) {
		int gt_x1 = gt[i].x1, gt_y1 = gt[i].y1;
		int gt_x2 = gt[i].x2, gt_y2 = gt[i].y2;
		double gt_a = (gt_x2-gt_x1) * (gt_y2-gt_y1);

		for (j = 0; j < n_c; j++) {
			int c_x1 = c[j].x1, c_y1 = c[j].y1;
			int c_x2 = c[j].x2, c_y2 = c[j].y2;
			double c_a = (c_x2-c_x1) * (c_y2-c_y1);
			double pro_a;
			double d1, d2;

			/* 共通部分の面積pro_aの計算 */
			int x1, y1;
			int x2, y2;
			x1 = (gt_x1 > c_x1) ? gt_x1 : c_x1;
			y1 = (gt_y1 > c_y1) ? gt_y1 : c_y1;
			x2 = (gt_x2 < c_x2) ? gt_x2 : c_x2;
			y2 = (gt_y2 < c_y2) ? gt_y2 : c_y2;

			/* 最小矩形の重なり判定 */
			if (gt_x1 > c_x2 || gt_x2 < c_x1 || gt_y1 > c_y2 || gt_y2 < c_y1) {
				pro_a = 0.0;
			} else {
				pro_a = (x2-x1) * (y2-y1);
			}

			d1 = sqrt((gt_x1-c_x1)*(gt_x1-c_x1) + (gt_y1-c_y1)*(gt_y1-c_y1));
			d2 = sqrt((gt_x2-c_x2)*(gt_x2-c_x2) + (gt_y2-c_y2)*(gt_y2-c_y2));

			/* 共通部分の面積が正解最小矩形のgt_r倍以上かつ最小矩形のc_r倍以上 */
			/* の場合に対応する最小矩形の候補としcand_listに距離を保存         */
			if (pro_a > gt_r * gt_a && pro_a > c_r * c_a) cand_list[i][j] = d1+d2;
			else                                          cand_list[i][j] =  -1.0;
		}
	}
}


/*------------------------------------------------------------*/
/* バックトラックによる最適な対応付け                         */
/*------------------------------------------------------------*/
static double backtrack(double **cand, int n_gt, int n_c, int curr, int *used, double d, double min_d, int *sol)
{
	int i;
	double dist, m;

	if (curr == n_gt) return d;

	for (i = 0; i < n_c; i++) {
		if (cand[curr][i] >= 0.0 && !used[i]) {
			dist = d + cand[curr][i];
			if (dist > min_d) continue;

			used[i] = 1;
			m = backtrack(cand, n_gt, n_c, curr+1, used, dist, min_d, sol);
			used[i] = 0;

			if (m < min_d) {
				min_d = m;
				sol[curr] = i;
			}
		}
	}

	dist = d + PENALTY;
	if (dist < min_d) {
		m = backtrack(cand, n_gt, n_c, curr+1, used, dist, min_d, sol);

		if (m < min_d) {
			min_d = m;
			sol[curr] = -1;
		}
	}

	return min_d;
}


/*------------------------------------------------------------*/
/* 正解最小矩形と結果の最小矩形の左上座標、右上座標それぞれの */
/* 距離の和が最小になるよう正解と結果の最小矩形を対応付け     */
/*------------------------------------------------------------*/
static double find_correspondent_object(double **d, int n_gt, int n_c, int *sol)
{
	int i;
	int *used;
	double min;

	used = (int *)malloc(n_c*sizeof(int));
	for (i = 0; i < n_c; i++) used[i] = 0;

	min = backtrack(d, n_gt, n_c, 0, used, 0, DBL_MAX, sol);

	free(used);

	return min;
}


/*------------------------------------------------------------*/
/* 正解ファイルの読み込み                                     */
/*------------------------------------------------------------*/
static object *read_ground_truth(const char *filename, int *n)
{
	int c, i;
	object *obj;

	FILE *f = fopen(filename, "r");
	fscanf(f, "%d\n", &c);

	obj = (object *)malloc(c * sizeof(object));

	for (i = 0; i < c; i++) {
		fscanf(f, "%d %d %d %d %d\n", &obj[i].x1, &obj[i].y1, &obj[i].x2, &obj[i].y2, &obj[i].label);
	}

	*n = c;

	fclose(f);

	return obj;
}


/*------------------------------------------------------------*/
/* ラベルの対応と代表例を基にF値を計算                        */
/*------------------------------------------------------------*/
static int *calculate_f_measure(object *gt, int n_gt, object *c, int n_c, int *corr, double *F, int *n_lab, int *n_cls)
{
	int i, j, k;
	int *ic;
	int *class_corr;
	int **num;
	int n_class;
	int n_label;
	double *f;
	

	/* 正解ラベル数 */
	n_class = 0;
	for (i = 0; i < n_gt; i++) {
		if (n_class < gt[i].label) n_class = gt[i].label;
	}
	n_class += 1;

	/* ラベルの最大値 */
	n_label = 0;
	for (j = 0; j < n_c; j++) {
		if (c[j].label > n_label) n_label = c[j].label;
	}
	n_label += 1;

	/* アルゴリズムの出力から正解を見つける配列を作る */
	ic = (int *)malloc(n_c * sizeof(int));
	for (j = 0; j < n_c ; j++) ic[j] = -1;
	for (i = 0; i < n_gt; i++) {
		if (corr[i] >= 0) ic[corr[i]] = i;
	}

	/* 代表例から各ラベルに対応する正解ラベルを対応付ける */
	class_corr = (int *)malloc((n_label) * sizeof(int));
	for (i = 0; i < n_label; i++)  class_corr[i] = -1;

	for (j = 0; j < n_c; j++) {
		if (c[j].rep && ic[j] >= 0)
			class_corr[c[j].label] = gt[ic[j]].label;
	}

	/* クロス表を作る */
	num = (int **)malloc((n_class+1) * sizeof(int *));
	for (i = 0; i < n_class+1; i++) {
		num[i] = (int *)malloc((n_label+1) * sizeof(int));
		for (j = 0; j < n_label+1; j++) num[i][j] = 0;
	}

	for (j = 0; j < n_c; j++) {
		int a = (ic[j] == -1) ? n_class : gt[ic[j]].label;
		int b = c[j].label;
		num[a][b] += 1;
	}
	
	for (i = 0; i < n_gt; i++) {
		if (corr[i] == -1) num[gt[i].label][n_label] += 1;
	}

	/* クロス表に基づいてF値を計算                        */
	/* 複数のラベルが同じ正解ラベルに対応している場合は   */
	/* F値が大きくなるほうを選ぶ                          */
	f = (double *)malloc(n_class * sizeof(double));	
	for (i = 0; i < n_class; i++) f[i] = 0.0;
	for (i = 0; i < n_label; i++) {
		double f_meas;
		double pre, rec;
		int n_p = 0;
		int n_r = 0;
		int c_class = class_corr[i];

		if (c_class == -1) continue;
		for (k = 0; k < n_class+1; k++) n_p += num[k][i];
		for (k = 0; k < n_label+1; k++) n_r += num[c_class][k];

		if (n_p) pre = num[c_class][i] / (double)n_p;
		else     pre = 0.0;

		if (n_r) rec = num[c_class][i] / (double)n_r;
		else     rec = 0.0;

		if (pre+rec < EPSILON) f_meas = 0.0;
		else                   f_meas = 2.0*pre*rec / (pre+rec);

		if (f[c_class] < f_meas) {
			f[c_class] = f_meas;
			for (j = 0; j < i; j++) {
				if (class_corr[j] == c_class) {
					class_corr[j] = -2;
				}
			}
		}
		else class_corr[i]= -2 ; 
	}
	
	/* F値の加重平均を算出 */
	*F = 0.0;
	for (i = 0; i < n_class; i++){ 
		int n_r	= 0; 
		for (j = 0; j < n_label; j++) n_r += num[i][j]; 
		*F += f[i] * n_r / (double)n_gt;
	}

	for (i = 0; i < n_class+1; i++) free(num[i]);
	free(num);
	free(ic);
	free(f);
	

	*n_lab = n_label;
	*n_cls = n_class;
	return class_corr;
}


void evaluate(object *c, int n_c, const char *filename)
{
	int i, j;
	int n_class;
	int n_label;
	int *corr;
	int *class_result;
	double **d;
	double min;
	double F;
	int *class_corr;
	object *gt;
	int n_gt;


	gt = read_ground_truth(filename, &n_gt);
	
	d = (double **)malloc(n_gt*sizeof(double *));
	for (i = 0; i < n_gt; i++) d[i] = (double *)malloc(n_c*sizeof(double));
	corr = (int *)malloc(n_gt*sizeof(int));

	create_candidate_list(gt, n_gt, c, n_c, d);

	/* 正解最小矩形と結果の最小矩形を対応付ける */
	min = find_correspondent_object(d, n_gt, n_c, corr);

	/* F値を計算 */
	class_corr = calculate_f_measure(gt, n_gt, c, n_c, corr, &F, &n_label, &n_class);
	
	/* 結果表示用の正解ラベル */
#if 0 /* 2009/5/8 修正 */
	class_result = (int *)malloc(n_c * sizeof(int));
#endif
	class_result = (int *)malloc(n_class * sizeof(int));
	for (i = 0; i < n_class; i++) class_result[i] = -1;
	for (i = 0; i < n_label; i++) {
		if (class_corr[i] >= 0) { 
			class_result[class_corr[i]] = i;
		}
	}
	j = n_label;
	for(i = 0; i < n_class; i++){
		if(class_result[i] == -1){
			class_result[i] = j;
			j++;
		}
	}
	for (i = 0; i < n_gt; i++) gt[i].label = class_result[gt[i].label];
		
		
	/* 結果の表示 */
	printf("\n");
	printf("           抽出矩形                             正解矩形\n");
	printf("     左上座標     右下座標   ラベル  ->   左上座標     右下座標  　ラベル  正誤\n");
	for (i = 0; i < n_c; i++) {
		int rep = (c[i].rep) ? 'o' : ' ';
		int g = -1;

		for (j = 0; j < n_gt; j++) {
			if (corr[j] == i) g = j;
		}
		 
		printf("%c (%4d, %4d) (%4d, %4d)    %d     -> ", rep, c[i].x1, c[i].y1, c[i].x2, c[i].y2, c[i].label);
		if (g == -1) {
			printf("(----, ----) (----, ----)    -\n");
		} else {			
			printf("(%4d, %4d) (%4d, %4d)    %d", gt[g].x1, gt[g].y1, gt[g].x2, gt[g].y2, gt[g].label);
		}
		if (g >= 0 && c[i].label==gt[g].label) printf("      o\n");
		else printf("      x\n");
	}	
	for (i = 0; i < n_gt; i++) {
		if(corr[i] == -1){
			printf("  (----, ----) (----, ----)    -     ->");
			printf(" (%4d, %4d) (%4d, %4d)    %d      x\n", gt[i].x1, gt[i].y1, gt[i].x2, gt[i].y2, gt[i].label);
		}
	}
	printf("\n");
	
	
	printf("得点:  %lf\n", F*100);

	for (i = 0; i < n_gt; i++) free(d[i]);
	free(d);
	free(corr);
	free(gt);
	free(class_result);
	free(class_corr);
}
