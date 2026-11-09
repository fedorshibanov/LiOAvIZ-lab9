#include "graph.h"


/* DFS повторно раскрывает вершину при улучшении расстояния.
   Обычный DFS первого посещения не гарантирует кратчайших путей. */
void dfs_relax(const Graph *g,int u,bool lists,int *d) {
    if(lists) {
        for(Node *p=g->adj[u];p;p=p->next) {
            int v=p->vertex;
            if(d[v]<0 || d[v]>d[u]+1) {d[v]=d[u]+1; dfs_relax(g,v,lists,d);}
        }
    } else for(int v=0;v<g->n;++v) if(g->a[u*g->n+v] && (d[v]<0 || d[v]>d[u]+1)) {
        d[v]=d[u]+1; dfs_relax(g,v,lists,d);
    }
}
/* 0 BFS матрица, 1 BFS списки, 2 DFS матрица, 3 DFS списки. */
void distances(const Graph *g,int start,int mode,int *d) {
    for(int i=0;i<g->n;++i) d[i]=-1;
    d[start]=0;
    if(mode>=2) {dfs_relax(g,start,mode==3,d); return;}
    std::queue<int> q; q.push(start);
    while(!q.empty()) {
        int u=q.front(); q.pop();
        if(mode==1) {
            for(Node *p=g->adj[u];p;p=p->next) if(d[p->vertex]<0) {
                d[p->vertex]=d[u]+1; q.push(p->vertex);
            }
        } else for(int v=0;v<g->n;++v) if(g->a[u*g->n+v] && d[v]<0) {
            d[v]=d[u]+1; q.push(v);
        }
    }
}

void benchmark() {
    puts("Замеры: среднее 10 поисков; p=10%; Время CPU, мс.");
    puts("n; bfs_matrix_ms; bfs_list_ms; dfs_matrix_ms; dfs_list_ms");
    int sizes[]={50,100,200,400};
    long checksum=0;
    for(int i=0;i<4;++i) {
        int n=sizes[i];
        Graph g=generate(n,10,0,0);
        int *d=(int *)allocate(n,sizeof(int));
        printf("%d",n);
        for(int mode=0;mode<4;++mode) {
            double start=now_ms();
            for(int r=0;r<10;++r) {
                distances(&g,0,mode,d); checksum+=d[n-1];
            }
            printf("; %.6f",(now_ms()-start)/10);
        }
        puts(""); free(d); graph_free(&g);
    }
    printf("Контрольная сумма: %ld\n",checksum);
}
int main(int argc,char **argv) {
    srand((unsigned)time(NULL));
    Options o=options(argc,argv);
    if(o.bench) {benchmark(); return 0;}
    Graph g=load_graph(&o,false,false);
    printf("ЛР9. Расстояния. n=%d, p=%d%%, start=%d\n",g.n,o.p,o.start);
    print_matrix(&g); print_lists(&g);
    int *d=(int *)allocate(g.n,sizeof(int));
    const char *names[]={"BFS матрица:","BFS списки:","DFS матрица с релаксацией:","DFS списки с релаксацией:"};
    for(int mode=0;mode<4;++mode) {
        distances(&g,o.start-1,mode,d); print_vector(names[mode],d,g.n);
    }
    puts("-1 означает недостижимую вершину.");
    free(d); graph_free(&g); return 0;
}
