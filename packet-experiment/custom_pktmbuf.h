static int custom_pktmbuf(void){

  struct rte_mbuf *m = NULL;
  char *data;

  m = rte_pktmbuf_alloc(pktmbuf_pool);

  // create packet with size 50
  data = rte_pktmbuf_append(m, 50);
  printf("length: %d\n", rte_pktmbuf_pkt_len(m));


  // increase packet size to 100
  data = rte_pktmbuf_prepend(m, 50);
  printf("length: %d\n", rte_pktmbuf_pkt_len(m));



	// increase packet size to 300
  // decrease packet size to 250
  // increase packet size to 300




  return 0;

}
