int gui_value1;
int gui_value2;
int gui_value3;
int gui_value_array[5] = {1, 2, 3, 4, 5};

void main(){
	gui_value1 = 1;
	gui_value2 = 2;
	gui_value3 = 3;
	
	char mem_loc_str[100];
	printf("Enter a memory location: ");
	fgets(mem_loc_str, sizeof mem_loc_str, stdin);
	
	char mem_size_str[100];
	printf("Enter number of integers to read: ");
	fgets(mem_size_str, sizeof mem_size_str, stdin);
	
	int * p = (int *)strtol(mem_loc_str, NULL, 16);
	int mem_size = atoi(mem_size_str);
	
	int i;
	printf("The value at that memory location is [");
	for(i = 0; i < mem_size - 1; i++){
		printf("%d, ", (*(p+1)));
	}
	printf("%d ]\n", (*(p+1)));
}