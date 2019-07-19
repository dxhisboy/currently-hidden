#include <data.h>
#include <multiproc.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <pair_lj.h>
#include <thermo.h>
#include <timer.h>
//#define DEBUG_THIS_FILE
#include <log.h>
void initial_integrate_nve(esmd_t *md){
  box_t *box = &(md->box);
  integrate_conf_t *integrate_conf = &(md->integrate_conf);
  pair_conf_t *pair_conf = &(md->pair_conf);
  areal dt = integrate_conf->dt;
  areal dtf = dt * 0.5;
  areal *rmass = pair_conf->rmass;
  double totalv = 0;
  for (int kk = 0; kk < box->nlocal[2]; kk ++){
    for (int jj = 0; jj < box->nlocal[1]; jj ++){
      for (int ii = 0; ii < box->nlocal[0]; ii ++){


        int cell_off = get_cell_off(box, ii, jj, kk);
        cell_t *cell = box->cells + cell_off;
        celldata_t *data = box->celldata + cell_off;
        areal (*v)[3] = data->v;
        areal (*x)[3] = data->x;
        areal (*f)[3] = data->f;
        int *type = data->type;
        for (int i = 0; i < cell->natoms; i ++){
          totalv += fabs(v[i][0]) + fabs(v[i][1]) + fabs(v[i][2]);
          v[i][0] += dtf * f[i][0] * rmass[type[i]];
          v[i][1] += dtf * f[i][1] * rmass[type[i]];
          v[i][2] += dtf * f[i][2] * rmass[type[i]];
          x[i][0] += dt * v[i][0];
          x[i][1] += dt * v[i][1];
          x[i][2] += dt * v[i][2];
        }
      }
    }
  }
  //debug("totalv = %g\n", totalv);
}

void final_integrate_nve(esmd_t *md){
  box_t *box = &(md->box);
  integrate_conf_t *integrate_conf = &(md->integrate_conf);
  pair_conf_t *pair_conf = &(md->pair_conf);
  areal dt = integrate_conf->dt;
  areal dtf = dt * 0.5;
  areal *rmass = pair_conf->rmass;
  double totalv = 0;
  for (int kk = 0; kk < box->nlocal[2]; kk ++){
    for (int jj = 0; jj < box->nlocal[1]; jj ++){
      for (int ii = 0; ii < box->nlocal[0]; ii ++){
	int cell_off = get_cell_off(box, ii, jj, kk);
	cell_t *cell = box->cells + cell_off;
	celldata_t *data = box->celldata + cell_off;
	areal (*v)[3] = data->v;
	areal (*f)[3] = data->f;
	int *type = data->type;
	for (int i = 0; i < cell->natoms; i ++){
	  v[i][0] += dtf * f[i][0] * rmass[type[i]];
	  v[i][1] += dtf * f[i][1] * rmass[type[i]];
	  v[i][2] += dtf * f[i][2] * rmass[type[i]];
	  totalv += fabs(v[i][0]) + fabs(v[i][1]) + fabs(v[i][2]);
	}
      }
    }
  }
  debug("totalv = %g\n", totalv);
}

#include <data.h>
void esmd_export_atoms(esmd_t *md){
  box_t *box = &(md->box);
  areal *rlcell = box->rlcell;
  integrate_conf_t *integrate_conf = &(md->integrate_conf);
  pair_conf_t *pair_conf = &(md->pair_conf);
  areal dt = integrate_conf->dt;
  areal dtf = dt * 0.5;
  areal *rmass = pair_conf->rmass;
  int total_export = 0;
  for (int kk = 0; kk < box->nlocal[2]; kk ++){
    for (int jj = 0; jj < box->nlocal[1]; jj ++){
      for (int ii = 0; ii < box->nlocal[0]; ii ++){

	int cell_off = get_cell_off(box, ii, jj, kk);
	cell_t *cell = box->cells + cell_off;
	celldata_t *data = box->celldata + cell_off;
	areal (*v)[3] = data->v;
	areal (*x)[3] = data->x;
	areal (*f)[3] = data->f;
	areal *q = data->q;
	int *export = data->export;
	int *type = data->type;

	int natoms_new = 0, export_ptr = CELL_SIZE;
	for (int i = 0; i < cell->natoms; i ++){
	  /* if (x[i][0] < 0) x[i][0] += box->lglobal[0]; */
	  /* if (x[i][1] < 0) x[i][1] += box->lglobal[1]; */
	  /* if (x[i][2] < 0) x[i][2] += box->lglobal[2]; */
	  /* if (x[i][0] > box->lglobal[0]) x[i][0] -= box->lglobal[0]; */
	  /* if (x[i][1] > box->lglobal[1]) x[i][1] -= box->lglobal[1]; */
	  /* if (x[i][2] > box->lglobal[2]) x[i][2] -= box->lglobal[2]; */
	  int cellx = floor(x[i][0] * rlcell[0] + TINY) - box->offset[0];
	  int celly = floor(x[i][1] * rlcell[1] + TINY) - box->offset[1];
	  int cellz = floor(x[i][2] * rlcell[2] + TINY) - box->offset[2];
	  /* if (cellx >= box->nglobal[0]) cellx -= box->nglobal[0]; */
	  /* if (celly >= box->nglobal[1]) celly -= box->nglobal[1]; */
	  /* if (cellz >= box->nglobal[2]) cellz -= box->nglobal[2]; */
	  /* if (cellx < 0) cellx += box->nglobal[0]; */
	  /* if (celly < 0) celly += box->nglobal[1]; */
	  /* if (cellz < 0) cellz += box->nglobal[2]; */
	  int new_cell_off = get_cell_off(box, cellx, celly, cellz);
	  int i_new;
	  if (cell_off != new_cell_off){
	    export_ptr --;
	    i_new = export_ptr;
	    export[i_new] = new_cell_off;
	  } else {
	    i_new = natoms_new;
	    natoms_new ++;
	  }
	  x[i_new][0] = x[i][0];
	  x[i_new][1] = x[i][1];
	  x[i_new][2] = x[i][2];
	  f[i_new][0] = f[i][0];
	  f[i_new][1] = f[i][1];
	  f[i_new][2] = f[i][2];
	  v[i_new][0] = v[i][0];
	  v[i_new][1] = v[i][1];
	  v[i_new][2] = v[i][2];
	  type[i_new] = type[i];
	  q[i_new] = q[i];
	}
	/* int nexport = CELL_SIZE - export_ptr; */
	/* for (int i = 0; i < nexport; i ++){ */
	/*   x[natoms_new + i][0] = x[export_ptr + i][0]; */
	/*   x[natoms_new + i][1] = x[export_ptr + i][1]; */
	/*   x[natoms_new + i][2] = x[export_ptr + i][2]; */
	/*   f[natoms_new + i][0] = f[export_ptr + i][0]; */
	/*   f[natoms_new + i][1] = f[export_ptr + i][1]; */
	/*   f[natoms_new + i][2] = f[export_ptr + i][2]; */
	/*   v[natoms_new + i][0] = v[export_ptr + i][0]; */
	/*   v[natoms_new + i][1] = v[export_ptr + i][1]; */
	/*   v[natoms_new + i][2] = v[export_ptr + i][2]; */
	/*   type[natoms_new + i] = type[export_ptr + i]; */
	/*   q[natoms_new + i] = q[export_ptr + i]; */
	/* } */
	/* memcpy(x[natoms_new], x[export_ptr], nexport * sizeof(areal) * 3); */
	/* memcpy(f[natoms_new], f[export_ptr], nexport * sizeof(areal) * 3); */
	/* memcpy(v[natoms_new], v[export_ptr], nexport * sizeof(areal) * 3); */
	/* memcpy(q + natoms_new, q + export_ptr, nexport * sizeof(ireal)); */
	/* memcpy(type + natoms_new, type + export_ptr, nexport * sizeof(int)); */
	/* memcpy(export + natoms_new, export + export_ptr, nexport * sizeof(int)); */
	total_export += CELL_SIZE - export_ptr;
	cell->natoms = natoms_new;
	//cell->nexport = nexport;
	cell->export_ptr = export_ptr;
	//assert(export_ptr >= natoms_new + nexport);
      }
    }
  }
  debug("%d\n", total_export);
}

int esmd_import_atoms_pairwise(box_t *box, int self_off, int neigh_off) {
  cell_t *cell_self = box->cells + self_off;
  celldata_t *data_self = box->celldata + self_off;
  cell_t *cell_neigh = box->cells + neigh_off;
  celldata_t *data_neigh = box->celldata + neigh_off;
  areal *rlcell = box->rlcell;
  areal (*v)[3] = data_self->v;
  areal (*x)[3] = data_self->x;
  areal (*f)[3] = data_self->f;
  areal *q = data_self->q;
  int *type = data_self->type;

  areal (*ve)[3] = data_neigh->v;
  areal (*xe)[3] = data_neigh->x;
  areal (*fe)[3] = data_neigh->f;
  areal *qe = data_neigh->q;
  int *typee = data_neigh->type;
  int *export = data_neigh->export;
  int natoms_new = cell_self->natoms;
  for (int i = cell_neigh->export_ptr; i < CELL_SIZE; i ++){
    //for (int i = cell_neigh->natoms; i < cell_neigh->natoms + cell_neigh->nexport; i ++){
    //debug("%d %d\n", cell_neigh->nexport, CELL_SIZE - cell_neigh->export_ptr);
    areal xe1 = xe[i][0], xe2 = xe[cell_neigh->export_ptr + i - cell_neigh->natoms][0];
    //if (xe1 != xe2) debug("%d %d %d %f %f\n", i - cell_neigh->natoms, box->celltype[self_off], box->celltype[neigh_off], xe1, xe2);
    int cellx = floor(xe[i][0] * rlcell[0] + TINY) - box->offset[0];
    int celly = floor(xe[i][1] * rlcell[1] + TINY) - box->offset[1];
    int cellz = floor(xe[i][2] * rlcell[2] + TINY) - box->offset[2];
    int import_off = get_cell_off(box, cellx, celly, cellz);
    if (import_off == self_off){
      x[natoms_new][0] = xe[i][0];
      x[natoms_new][1] = xe[i][1];
      x[natoms_new][2] = xe[i][2];
      int safe = (xe[i][0] >= cell_self->bbox_ideal[0][0] && xe[i][0] <= cell_self->bbox_ideal[1][0] &&
		  xe[i][1] >= cell_self->bbox_ideal[0][1] && xe[i][1] <= cell_self->bbox_ideal[1][1] &&
		  xe[i][2] >= cell_self->bbox_ideal[0][2] && xe[i][2] <= cell_self->bbox_ideal[1][2]);
      if (!safe){
	debug("%d unsafely imported %f %f %f\n", self_off, x[natoms_new][0], x[natoms_new][1], x[natoms_new][2]);
      }
 
      f[natoms_new][0] = fe[i][0];
      f[natoms_new][1] = fe[i][1];
      f[natoms_new][2] = fe[i][2];
      v[natoms_new][0] = ve[i][0];
      v[natoms_new][1] = ve[i][1];
      v[natoms_new][2] = ve[i][2];
      type[natoms_new] = typee[i];
      q[natoms_new] = qe[i];
      natoms_new ++;
    }
  }
  int nimport = natoms_new - cell_self->natoms;
  cell_self->natoms = natoms_new;
  return nimport;
}

void esmd_import_atoms_outer_from_local(esmd_t *md){
  int nimport = 0;
  box_t *box = &(md->box);
  int *nlocal = box->nlocal;
  int *celltype = box->celltype;
  for (int kk = 0; kk < nlocal[2]; kk ++){
    for (int jj = 0; jj < nlocal[1]; jj ++){
      for (int ii = 0; ii < nlocal[0]; ii ++){
	int cell_off = get_cell_off(box, ii, jj, kk);
	if (celltype[cell_off] != CT_OUTER) continue;
	for (int dz = -1; dz <= 1; dz ++){
	  for (int dy = -1; dy <= 1; dy ++){
	    for (int dx = -1; dx <= 1; dx ++){

	      int neigh_off = get_cell_off(box, ii + dx, jj + dy, kk + dz);
	      if (celltype[neigh_off] == CT_HALO) continue;
	      nimport += esmd_import_atoms_pairwise(box, cell_off, neigh_off);
	    }
	  }
	}
	
      }
    }
  }
  debug("%d outer from local %d\n", md->mpp.pid, nimport);
}

void esmd_import_atoms_outer_from_halo(esmd_t *md){
  int nimport = 0;
  box_t *box = &(md->box);
  int *nlocal = box->nlocal;
  int *celltype = box->celltype;
  for (int kk = 0; kk < nlocal[2]; kk ++){
    for (int jj = 0; jj < nlocal[1]; jj ++){
      for (int ii = 0; ii < nlocal[0]; ii ++){
	int cell_off = get_cell_off(box, ii, jj, kk);
	if (celltype[cell_off] != CT_OUTER) continue;

	for (int dz = -1; dz <= 1; dz ++){
	  for (int dy = -1; dy <= 1; dy ++){
	    for (int dx = -1; dx <= 1; dx ++){
	      int neigh_off = get_cell_off(box, ii + dx, jj + dy, kk + dz);
	      if (celltype[neigh_off] != CT_HALO) continue;
	      nimport += esmd_import_atoms_pairwise(box, cell_off, neigh_off);
	    }
	  }
	}
	
      }
    }
  }
  debug("%d outer from halo %d\n", md->mpp.pid, nimport);
}

void esmd_import_atoms_inner_from_local(esmd_t *md){
  int nimport = 0;
  box_t *box = &(md->box);
  int *nlocal = box->nlocal;
  int *celltype = box->celltype;
  for (int kk = 0; kk < nlocal[2]; kk ++){
    for (int jj = 0; jj < nlocal[1]; jj ++){
      for (int ii = 0; ii < nlocal[0]; ii ++){
	int cell_off = get_cell_off(box, ii, jj, kk);
	if (celltype[cell_off] != CT_INNER) continue;
	for (int dz = -1; dz <= 1; dz ++){
	  for (int dy = -1; dy <= 1; dy ++){
	    for (int dx = -1; dx <= 1; dx ++){
	      int neigh_off = get_cell_off(box, ii + dx, jj + dy, kk + dz);
	      nimport += esmd_import_atoms_pairwise(box, cell_off, neigh_off);
	    }
	  }
	}
	
      }
    }
  }
  debug("%d inner from local %d\n", md->mpp.pid, nimport);
}

int get_halo_type(box_t *box, int ii, int jj, int kk){
  int ret = 13;
  if (ii < 0) ret -= 9;
  if (ii >= box->nlocal[0]) ret += 9;
  if (jj < 0) ret -= 3;
  if (jj >= box->nlocal[1]) ret += 3;
  if (kk < 0) ret -= 1;
  if (kk >= box->nlocal[2]) ret += 1;
  return ret;
}
void esmd_import_atoms_halo_from_local(esmd_t *md){
  int nimport = 0;
  box_t *box = &(md->box);
  int *nlocal = box->nlocal;
  int *celltype = box->celltype;
  for (int kk = -NCELL_CUT; kk < nlocal[2] + NCELL_CUT; kk ++){
    for (int jj = -NCELL_CUT; jj < nlocal[1] + NCELL_CUT; jj ++){
      for (int ii = -NCELL_CUT; ii < nlocal[0] + NCELL_CUT; ii ++){
	int cell_off = get_cell_off(box, ii, jj, kk);
	if (celltype[cell_off] != CT_HALO) continue;
	for (int dz = -1; dz <= 1; dz ++){
	  for (int dy = -1; dy <= 1; dy ++){
	    for (int dx = -1; dx <= 1; dx ++){
	      if (ii + dx >= -NCELL_CUT && ii + dx < nlocal[0] + NCELL_CUT &&
		  jj + dy >= -NCELL_CUT && jj + dy < nlocal[1] + NCELL_CUT &&
		  kk + dz >= -NCELL_CUT && kk + dz < nlocal[2] + NCELL_CUT){
		if (get_halo_type(box, ii, jj, kk) == get_halo_type(box, ii + dx, jj + dy, kk + dz)) continue;
		int neigh_off = get_cell_off(box, ii + dx, jj + dy, kk + dz);
		nimport += esmd_import_atoms_pairwise(box, cell_off, neigh_off);
	      }
	    }
	  }
	}
	
      }
    }
  }
  debug("halo from local %d\n", nimport);
}

void esmd_import_atoms(esmd_t *md){
  box_t *box = &(md->box);
  for (int kk = 0; kk < box->nlocal[2]; kk ++){
    for (int jj = 0; jj < box->nlocal[1]; jj ++){
      for (int ii = 0; ii < box->nlocal[0]; ii ++){
	int cell_off = get_cell_off(box, ii, jj, kk);
	for (int dz = -1; dz <= 1; dz ++) {
	  for (int dx = -1; dx <= 1; dx ++){
	    for (int dy = -1; dy <= 1; dy ++){
	      
	      int neigh_off = get_cell_off(box, ii + dx, jj + dy, kk + dz);
	      esmd_import_atoms_pairwise(box, cell_off, neigh_off);
	    }
	  }
	}
	
      }
    }
  }
}

void integrate(esmd_t *md) {
  timer_start("compute");
  
  compute_kinetic_local(md);
  esmd_global_accumulate(md);
  thermo_compute(md);
  
  timer_stop("compute");
  master_info("step: %d eng: %f temp: %f press: %f\n", md->step, md->thermo.eng, md->thermo.temp, md->thermo.press);
  
  md->accu_local.virial = 0;
  md->accu_local.epot = 0;
  md->accu_local.kinetic = 0;
  
  timer_start("init_integrate");
  initial_integrate_nve(md);
  timer_stop("init_integrate");
  
  timer_start("export");
  esmd_export_atoms(md);
  timer_stop("export");
  
  timer_start("comm_l2h");
  esmd_exchange_cell(md, LOCAL_TO_HALO, CELL_META | CELL_X | CELL_T | CELL_V, TRANS_ADJ_X | TRANS_EXPORTS);
  timer_stop("comm_l2h");
  
  timer_start("ac_import");
  esmd_import_atoms(md);
  timer_stop("ac_import");
  
  esmd_exchange_cell(md, LOCAL_TO_HALO, CELL_META | CELL_X | CELL_T | CELL_V, TRANS_ADJ_X | TRANS_ATOMS);
  
  timer_start("force");
  pair_lj_force(md);
  timer_stop("force");
  
  timer_start("comm_h2l");
  esmd_exchange_cell(md, HALO_TO_LOCAL, CELL_META | CELL_F, TRANS_INC_F | TRANS_ATOMS);
  timer_stop("comm_h2l");
  
  //esmd_exchange_cell(md, LOCAL_TO_HALO, CELL_F);
  timer_start("final_integrate");
  final_integrate_nve(md);
  timer_stop("final_integrate");
  md->step ++;
}
