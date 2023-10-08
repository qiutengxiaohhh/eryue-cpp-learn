#include <header.h>


std::vector<int> v;

void dfs(TreeNode* root) {
    if (root== nullptr) return;

    dfs(root->left);
    v.push_back(root->val);
    dfs(root->right);
}

bool isValidBST(TreeNode* root) {
    if(root== nullptr) return true;

    isValidBST(root);

    for (int i = 1; i < v.size(); ++i) {
        if(v[i] < v[i-1]) {
            return false;
        }
    }
    return true;


}

int main() {


}
