#ifndef GRAPH_H
#define GRAPH_H
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <climits>
#include <ctime>
#include <queue>

/* Матрица и настоящие односвязные списки смежности. Нумерация внутри с нуля. */
struct Node { int vertex; Node *next; };
struct Graph { int n; int *a; Node **adj; };
void *allocate(size_t n, size_t size) {
    void *p = calloc(n, size);
    if (!p) { fputs("Недостаточно памяти\n", stderr); exit(1); }
    return p;
}
Graph graph_new(int n) {
    Graph g = {n, (int *)allocate((size_t)n*n,sizeof(int)),
               (Node **)allocate(n,sizeof(Node *))};
    return g;
}
void edge(Graph *g,int u,int v,int w,int directed) {
    g->a[u*g->n+v]=w;
    if (!directed) g->a[v*g->n+u]=w;
}
void graph_lists(Graph *g) {
    for(int u=0;u<g->n;++u) {
        for(int v=g->n-1;v>=0;--v) if(g->a[u*g->n+v]) {
            Node *p=(Node *)allocate(1,sizeof(Node));
            p->vertex=v;
            p->next=g->adj[u];
            g->adj[u]=p;
        }
    }
}
void graph_free(Graph *g) {
    for(int i=0;i<g->n;++i) {
        Node *p=g->adj[i];
        while(p) { Node *next=p->next; free(p); p=next; }
    }
    free(g->adj); free(g->a);
}
Graph generate(int n,int probability,int directed,int weighted) {
    Graph g=graph_new(n);
    for(int u=0;u<n;++u) for(int v=0;v<n;++v) {
        if(u==v || (!directed && v<u)) continue;
        if((int)(rand()%100)<probability)
            edge(&g,u,v,weighted ? 1+(int)(rand()%9) : 1,directed);
    }
    graph_lists(&g); return g;
}
void print_matrix(const Graph *g) {
    puts("Матрица смежности:"); printf("    ");
    for(int i=0;i<g->n;++i) printf("%4d",i+1);
    puts("");
    for(int u=0;u<g->n;++u) {
        printf("%3d ",u+1);
        for(int v=0;v<g->n;++v) printf("%4d",g->a[u*g->n+v]);
        puts("");
    }
}
void print_lists(const Graph *g) {
    puts("Списки смежности:");
    for(int u=0;u<g->n;++u) {
        printf("%d:",u+1);
        for(Node *p=g->adj[u];p;p=p->next) printf(" %d",p->vertex+1);
        puts(" -> NULL");
    }
}
void print_vector(const char *label,const int *d,int n) {
    printf("%s",label);
    for(int i=0;i<n;++i) printf(" %d",d[i]);
    puts("");
}
struct Options {
    int n=6, p=40, start=1, directed=0, weighted=1;
    bool bench=false; const char *input=nullptr;
};
long number(const char *s,long low,long high) {
    char *end=nullptr; errno=0; long x=strtol(s,&end,10);
    if(errno || !*s || *end || x<low || x>high) {
        fprintf(stderr,"Недопустимое число: %s (ожидается %ld..%ld)\n",s,low,high); exit(1);
    }
    return x;
}
Options options(int argc,char **argv) {
    Options o;
    for(int i=1;i<argc;++i) {
        const char *s=argv[i];
        if(!strcmp(s,"--help")) {
            puts("--n 1..2000 --p 0..100 --start 1..n");
            puts("--directed 0|1 --weighted 0|1 --input matrix.txt --benchmark"); exit(0);
        }
        if(!strcmp(s,"--benchmark")) {o.bench=true; continue;}
        if(i+1==argc) {fprintf(stderr,"Нет значения: %s\n",s); exit(1);}
        const char *v=argv[++i];
        if(!strcmp(s,"--n")) o.n=(int)number(v,1,2000);
        else if(!strcmp(s,"--p")) o.p=(int)number(v,0,100);
        else if(!strcmp(s,"--start")) o.start=(int)number(v,1,2000);
        else if(!strcmp(s,"--directed")) o.directed=(int)number(v,0,1);
        else if(!strcmp(s,"--weighted")) o.weighted=(int)number(v,0,1);
        else if(!strcmp(s,"--input")) o.input=v;
        else {fprintf(stderr,"Неизвестный параметр: %s\n",s); exit(1);}
    }
    return o;
}
Graph load_graph(Options *o,bool weighted_allowed,bool directed_allowed) {
    if(!weighted_allowed) o->weighted=0;
    if(!directed_allowed) o->directed=0;
    if(!o->input) {
        if(o->start>o->n) {fputs("Начальная вершина вне графа\n",stderr); exit(1);}
        return generate(o->n,o->p,o->directed,o->weighted);
    }
    FILE *f=fopen(o->input,"r"); int n;
    if(!f) {perror(o->input); exit(1);}
    if(fscanf(f,"%d",&n)!=1 || n<1 || n>2000) {fputs("Неверный размер матрицы\n",stderr); exit(1);}
    Graph g=graph_new(n); o->n=n;
    for(int i=0;i<n;++i) for(int j=0;j<n;++j) {
        int w;
        if(fscanf(f,"%d",&w)!=1 || w<0 || w>100000 || (i==j && w) || (!o->weighted && w>1)) {
            fputs("Неверный элемент матрицы\n",stderr); exit(1);
        }
        g.a[i*n+j]=w;
    }
    char extra;
    if(fscanf(f," %c",&extra)==1) {fputs("Лишние данные в файле\n",stderr); exit(1);}
    fclose(f);
    for(int i=0;i<n;++i) for(int j=0;j<n;++j)
        if(!o->directed && g.a[i*n+j]!=g.a[j*n+i]) {fputs("Матрица несимметрична\n",stderr); exit(1);}
    if(o->start>n) {fputs("Начальная вершина вне графа\n",stderr); exit(1);}
    graph_lists(&g); return g;
}
double now_ms() {
    return 1000.0 * clock() / CLOCKS_PER_SEC;
}
#endif
