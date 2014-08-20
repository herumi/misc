# θΊ¦ιθ—(parallel)εγΞε¦―εβ°εγ©εγήε¦µεβ°

## ιο¤ιª©

* [parallelism-ts](https://github.com/cplusplus/parallelism-ts)

## νϋ®νϊ„

C++μªÒθΊΜε¦«εβ¤εγΜε¦«εγªεα«κ°ήλΣϋεα«λω®ιο΄ε΅χεα¦εα¨ε£λθΊ¦ιθΞε¥χεγ­εβ°εγ©εγήε¦µεβ°εβΔη°Όηª§εαÒε£λεΰ‚

## μ¨¤κ¦

* ιπΊη±νξ«ΊρφΕΑstd::experimental::parallel::v1`εα«ExecutionPolicyεγΒε¦«εγ΅εγΌεβΏεβΔθ·αεα¤ρφΆλυ°εγ¬ε¦µεγΞε¦®εγΌεγ°ε£ςπΑ½ικ εαÒε£λεΰ‚
* εγΠε¥γεγ€εγΚε¤£εβ¤εγ«εα―`meow`εΰ‚
* θΊ¦ιθΞε¤¤εγ«εβ΄εγªεβΊεγ εα―π¨Άι΄ εβΆεβ―εβ»εβΉρφΆλυ°ξ·ΈιΘ³εα§κΎΚθΚ²εα«εβΆεβ―εβ»εβΉεαÒε£λεΰ‚

## κ°ήκ΅Έε¥ύεγªεβ·εγΌ

θΐ‹

    std::vector<int> v = ...

    // λω®ρΰΤεΆ°εβ½εγΌεγ
    std::sort(v.begin(), v.end());

    // θΊ¦ιθΞι±θ
    using namespace std::experimental::parallel;

    // εβ·εγΌεβ±εγ³εβ·εγ£εγ«εβ½εγΌεγ(κΐΖθΪ§εα®εβ½εγΌεγ)εβΔθΟξξ¦Ί
    sort(seq, v.begin(), v.end());

    // θΊ¦ιθΞε¤ΏεγΌεγ°ε£ςπª±ιο―
    sort(par, v.begin(), v.end());

    // εγÒε¤±εγ°ε¦­ιμΜεΆªθΊ¦ιθΞε¤ΏεγΌεγ°ε£ςπª±ιο―
    sort(par_vec, v.begin(), v.end());

    // εγΪε¦¬εβ·εγΌεα®κ°ήκ΅ΈθΡβιθ®ε£κλοΦε΅θ
    size_t threshold = ...
    execution_policy exec = (v.size() > therashold) ? para : seq;
    sort(exec, v.begin(), v.end());


## εγΠε¥γεγ€`<experimental/execution_policy>`

    namespace std {
    namespace experimental {
    namespace parallel {
    inline namesapce v1 {

    template<class T> struct is_execution_policy;
    template<class T> constexpr bool is_execution_policy_v
      = is_execution_policy<T>::value;

     class sequential_execution_policy;
     class parallel_execution_policy;
     class parallel_vector_execution_policy;
     class execution_policy;
     }}}}

is_execution_policyεα―εα©εα®εγΪε¦¬εβ·εγΌεα®θΊ¦ιθΞη®ήκ΅Έε£ςπ£Έε΅ζεα¶ε£ςλμ®η®Τε΅ωεβ¶ε€‚

* sequential_execution_policy
    θΊ¦ιθΞη®ήκ΅Έε΅χεα¦εα―εα¨ε΅ρεαªεα¨ε΅σεα¨εβΔι¤Ίεα™
* parallel_execution_policy
    θΊ¦ιθΞη®ήκ΅Έε΅χεα¦εβ°ε΅δεαΖεΆªεβΔι¤Ίεα™
* parallel_vector_execution_policy
    εγÒε¤±εγ°ε¦­ιμΜεΆªθΊ¦ιθΞη®ήκ΅Έε΅χεα¦εβ°ε΅δεαΖεΆªεβΔι¤Ίεα™
* execution_policy
    κ°ήκ΅ΈθΡβεα«εγΪε¦¬εβ·εγΌεβΔθ±ΊεβΆε£ιεβΈε£λ

## θΊ¦ιθΞη®ήκ΅ΈθΡβεα®λμ―εβ¶κ―ώεα„

* κ°ήκ΅ΈζΈ­εα«κΑªκ¦ΆεΆ¬εγ΅εγΆεγªεαΈεΆ¬εαΒε£μεα°std::bad_allocεβΔθ³υεαΔε£λεΰ‚
* π¨Άι΄ εβΆεβ―εβ»εβΉρφΆλυ°εαΈζΎ¶η¤Με£ςιηΊεαΞεΆ΅εα¨εα
    * κ°ήκ΅Έε¥ύεγªεβ·εγΌεα·Ρarallel_vector_execution_policyεαªεβ±Τtd::terminateεβΔηΒΎεα¶εΰ‚
    * κ°ήκ΅Έε¥ύεγªεβ·εγΌεα·Τequential_execution_policyεαµΡarallel_execution_policyεαªεβ‰
      exception_listεβΔθ³υεαΔε£λεΰ‚
* θΊ¦ιθΞε¤¤εγ«εβ΄εγªεβΊεγ εα·Τtd::bad_allocεβΔθ³υεαΔεΆ¨ξ·¤ζΊ¬ε΅χεαήεΆ°εα§εαªεαΒε£μεα°εΰΆε΅ωεαΉεα¦εα®θΐ¶η¤ΜεΆ±ιρΌιηΊιε¦εΆ­θΎΪε΅θεβ²ε£μεβ¶ε€‚
  θΐ¶η¤Με΅μνωΊντήε΅χεαήε΅βεα¨εΰΆζΈ¦ιθΞε¤¤εγ«εβ΄εγªεβΊεγ εαΈε΅ύεα®εαΎεαΎρΰ²εβ€εα¶ε΅ύεα¬εΆ©εαªεα¨ε΅λεα―θΊΊη®Τε€‚

## exception_list

    class exception_list : public exception {
    public:
        typedef unspecified iterator;
        size_t size() const noexcept;
        iterator begin() const noexcept;
        iterator end() const noexcept;
        const char *what() const noexcept override;
    };

* iteratorεα―ForwardIterator;
* size()εα―exception_listεαΈθ·αεα¤εβªεγΜε¤Ίεβ§εβ―εγ°εΆ°ιΰ¶θΚ²
* begin(), end()εαΈε΅ύεα®εβªεγΜε¤Ίεβ§εβ―εγ°εΆ°ξ±¨ηΦ΄
* what()εα―θΏΚε΅λNTBS(NULLξ·¤ι«―λφ®η­Ξη―χ)εβΔκΏΘε΅ωεΰ‚
