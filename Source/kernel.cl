void kernel add(global const int* v1, global const int* v2, global int* v3) {
               int ID;
               ID = get_global_id(0);
               v3[ID] = v1[ID] + v1[ID];
}


